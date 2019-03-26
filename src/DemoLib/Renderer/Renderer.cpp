#include "Renderer.h"

#include <chrono>

#include <Ren/Context.h>
#include <Sys/Log.h>
#include <Sys/ThreadPool.h>

namespace RendererInternal {
bool bbox_test(const float p[3], const float bbox_min[3], const float bbox_max[3]);

extern const uint8_t bbox_indices[];

const bool USE_TWO_THREADS = true;
const int SHADOWMAP_WIDTH = 2048,
          SHADOWMAP_HEIGHT = 1024;

// Sun shadow occupies half of atlas
const int SUN_SHADOW_RES = SHADOWMAP_WIDTH / 2;
}

#define BBOX_POINTS(min, max) \
    (min)[0], (min)[1], (min)[2],     \
    (max)[0], (min)[1], (min)[2],     \
    (min)[0], (min)[1], (max)[2],     \
    (max)[0], (min)[1], (max)[2],     \
    (min)[0], (max)[1], (min)[2],     \
    (max)[0], (max)[1], (min)[2],     \
    (min)[0], (max)[1], (max)[2],     \
    (max)[0], (max)[1], (max)[2]

Renderer::Renderer(Ren::Context &ctx, std::shared_ptr<Sys::ThreadPool> &threads)
    : ctx_(ctx), threads_(threads), shadow_splitter_(RendererInternal::SHADOWMAP_WIDTH, RendererInternal::SHADOWMAP_HEIGHT) {
    using namespace RendererInternal;

    {
        const float NEAR_CLIP = 0.5f;
        const float FAR_CLIP = 10000.0f;
        SWfloat z = FAR_CLIP / (FAR_CLIP - NEAR_CLIP) + (NEAR_CLIP - (2.0f * NEAR_CLIP)) / (0.15f * (FAR_CLIP - NEAR_CLIP));
        swCullCtxInit(&cull_ctx_, 256, 128, z);
    }

    {   // Create shadow map buffer
        shadow_buf_ = FrameBuf(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, nullptr, 0, true, Ren::BilinearNoMipmap);

        // Reserve space for sun shadow
        const int sun_shadow_res[] = { SUN_SHADOW_RES, SUN_SHADOW_RES };
        int sun_shadow_pos[2];
        int id = shadow_splitter_.Allocate(sun_shadow_res, sun_shadow_pos);
        assert(id != -1 && sun_shadow_pos[0] == 0 && sun_shadow_pos[1] == 0);
    }

    {   // Create aux buffer which gathers frame luminance
        FrameBuf::ColorAttachmentDesc desc;
        desc.format = Ren::RawR16F;
        desc.filter = Ren::BilinearNoMipmap;
        desc.repeat = Ren::ClampToEdge;
        reduced_buf_ = FrameBuf(16, 8, &desc, 1, false);
    }

    // Compile built-in shadres etc.
    InitRendererInternal();

    uint8_t black[] = { 0, 0, 0, 0 }, white[] = { 255, 255, 255, 255 };

    Ren::Texture2DParams p;
    p.w = p.h = 1;
    p.format = Ren::RawRGBA8888;
    p.filter = Ren::Bilinear;

    Ren::eTexLoadStatus status;
    default_lightmap_ = ctx_.LoadTexture2D("default_lightmap", black, sizeof(black), p, &status);
    assert(status == Ren::TexCreatedFromData);

    default_ao_ = ctx_.LoadTexture2D("default_ao", white, sizeof(white), p, &status);
    assert(status == Ren::TexCreatedFromData);

    /*try {
        shadow_buf_ = FrameBuf(SHADOWMAP_RES, SHADOWMAP_RES, Ren::RawR32F, Ren::NoFilter, Ren::ClampToEdge, true);
    } catch (std::runtime_error &) {
        LOGI("Cannot create floating-point shadow buffer! Fallback to unsigned byte.");
        shadow_buf_ = FrameBuf(SHADOWMAP_RES, SHADOWMAP_RES, Ren::RawRGB888, Ren::NoFilter, Ren::ClampToEdge, true);
    }*/

    if (USE_TWO_THREADS) {
        background_thread_ = std::thread(std::bind(&Renderer::BackgroundProc, this));
    }

    for (int i = 0; i < 2; i++) {
        drawables_data_[i].cells.resize(CELLS_COUNT);
        drawables_data_[i].items.resize(MAX_ITEMS_TOTAL);
    }
}

Renderer::~Renderer() {
    if (background_thread_.joinable()) {
        shutdown_ = notified_ = true;
        thr_notify_.notify_all();
        background_thread_.join();
    }
    DestroyRendererInternal();
    swCullCtxDestroy(&cull_ctx_);
}

void Renderer::GatherObjects(const Ren::Camera &cam, const bvh_node_t *nodes, uint32_t root_index, uint32_t nodes_count,
                             const SceneObject *objects, const uint32_t *obj_indices, uint32_t object_count, const Environment &env,
                             const TextureAtlas &decals_atlas) {
    using namespace RendererInternal;

    if (USE_TWO_THREADS) {
        // Delegate gathering to background thread
        SwapDrawLists(cam, nodes, root_index, nodes_count, objects, obj_indices, object_count, env, &decals_atlas);
    } else {
        // Gather objects in main thread

        drawables_data_[0].draw_cam = cam;
        drawables_data_[0].env = env;

        GatherDrawables(nodes, root_index, objects, obj_indices, object_count, drawables_data_[0]);
    }
}

void Renderer::DrawObjects() {
    using namespace RendererInternal;

    uint64_t gpu_draw_start = 0;
    if (drawables_data_[0].render_flags & DebugTimings) {
        gpu_draw_start = GetGpuTimeBlockingUs();
    }
    auto cpu_draw_start = std::chrono::high_resolution_clock::now();
    
    {
        size_t transforms_count = drawables_data_[0].transforms.size();
        const auto *transforms = (transforms_count == 0) ? nullptr : &drawables_data_[0].transforms[0];

        size_t drawables_count = drawables_data_[0].draw_list.size();
        const auto *drawables = (drawables_count == 0) ? nullptr : &drawables_data_[0].draw_list[0];

        size_t lights_count = drawables_data_[0].light_sources.size();
        const auto *lights = (lights_count == 0) ? nullptr : &drawables_data_[0].light_sources[0];

        size_t decals_count = drawables_data_[0].decals.size();
        const auto *decals = (decals_count == 0) ? nullptr : &drawables_data_[0].decals[0];

        const auto *cells = drawables_data_[0].cells.empty() ? nullptr : &drawables_data_[0].cells[0];

        size_t items_count = drawables_data_[0].items_count;
        const auto *items = (items_count == 0) ? nullptr : &drawables_data_[0].items[0];

        const auto *p_decals_atlas = drawables_data_[0].decals_atlas;

        Ren::Mat4f shadow_transforms[4];
        size_t shadow_drawables_count[4];
        const DrawableItem *shadow_drawables[4];

        for (int i = 0; i < 4; i++) {
            Ren::Mat4f view_from_world = drawables_data_[0].shadow_cams[i].view_matrix(),
                       clip_from_view = drawables_data_[0].shadow_cams[i].proj_matrix();
            shadow_transforms[i] = clip_from_view * view_from_world;
            shadow_drawables_count[i] = drawables_data_[0].shadow_list[i].size();
            shadow_drawables[i] = (shadow_drawables_count[i] == 0) ? nullptr : &drawables_data_[0].shadow_list[i][0];
        }

        if (ctx_.w() != w_ || ctx_.h() != h_) {
            {   // Main buffer for raw frame before tonemapping
                FrameBuf::ColorAttachmentDesc desc[3];
                {   // Main color
                    desc[0].format = Ren::RawRGBA16F;
                    desc[0].filter = Ren::NoFilter;
                    desc[0].repeat = Ren::ClampToEdge;
                }
                {   // View-space normal
                    desc[1].format = Ren::RawRG88;
                    desc[1].filter = Ren::BilinearNoMipmap;
                    desc[1].repeat = Ren::ClampToEdge;
                }
                {   // 4-component specular (alpha is roughness)
                    desc[2].format = Ren::RawRGBA8888;
                    desc[2].filter = Ren::BilinearNoMipmap;
                    desc[2].repeat = Ren::ClampToEdge;
                }
                clean_buf_ = FrameBuf(ctx_.w(), ctx_.h(), desc, 3, true, Ren::NoFilter, 4);
            }
            {   // Buffer for SSAO
                FrameBuf::ColorAttachmentDesc desc;
                desc.format = Ren::RawR8;
                desc.filter = Ren::BilinearNoMipmap;
                desc.repeat = Ren::ClampToEdge;
                ssao_buf_ = FrameBuf(ctx_.w() / 2, ctx_.h() / 2, &desc, 1, false);
            }
            {   // Auxilary buffer for reflections
                FrameBuf::ColorAttachmentDesc desc;
                desc.format = Ren::RawRGB16F;
                desc.filter = Ren::Bilinear;
                desc.repeat = Ren::ClampToEdge;
                refl_buf_ = FrameBuf(ctx_.w() / 2, ctx_.h() / 2, &desc, 1, false);
            }
            {   // Buffer that holds previous frame (used for SSR)
                FrameBuf::ColorAttachmentDesc desc;
                desc.format = Ren::RawRGB16F;
                desc.filter = Ren::Bilinear;
                desc.repeat = Ren::ClampToEdge;
                down_buf_ = FrameBuf(ctx_.w() / 4, ctx_.h() / 4, &desc, 1, false);
            }
            {   // Auxilary buffers for bloom effect
                FrameBuf::ColorAttachmentDesc desc;
                desc.format = Ren::RawRGBA16F;
                desc.filter = Ren::BilinearNoMipmap;
                desc.repeat = Ren::ClampToEdge;
                blur_buf1_ = FrameBuf(ctx_.w() / 4, ctx_.h() / 4, &desc, 1, false);
                blur_buf2_ = FrameBuf(ctx_.w() / 4, ctx_.h() / 4, &desc, 1, false);
            }
            w_ = ctx_.w();
            h_ = ctx_.h();
            LOGI("CleanBuf resized to %ix%i", w_, h_);
        }

        drawables_data_[0].render_info.lights_count = (uint32_t)lights_count;
        drawables_data_[0].render_info.lights_data_size = (uint32_t)lights_count * sizeof(LightSourceItem);
        drawables_data_[0].render_info.decals_count = (uint32_t)decals_count;
        drawables_data_[0].render_info.decals_data_size = (uint32_t)decals_count * sizeof(DecalItem);
        drawables_data_[0].render_info.cells_data_size = (uint32_t)CELLS_COUNT * sizeof(CellData);
        drawables_data_[0].render_info.items_data_size = (uint32_t)drawables_data_[0].items.size() * sizeof(ItemData);

        DrawObjectsInternal(drawables_data_[0]);
    }
    
    auto cpu_draw_end = std::chrono::high_resolution_clock::now();

    // store values for current frame
    backend_cpu_start_ = (uint64_t)std::chrono::duration<double, std::micro>{ cpu_draw_start.time_since_epoch() }.count();
    backend_cpu_end_ = (uint64_t)std::chrono::duration<double, std::micro>{ cpu_draw_end.time_since_epoch() }.count();
    backend_time_diff_ = int64_t(gpu_draw_start) - int64_t(backend_cpu_start_);
    frame_counter_++;
}

void Renderer::WaitForBackgroundThreadIteration() {
    SwapDrawLists(drawables_data_[0].draw_cam, nullptr, 0, 0, nullptr, nullptr, 0, drawables_data_[0].env, nullptr);
}

void Renderer::BackgroundProc() {
    using namespace RendererInternal;

    std::unique_lock<std::mutex> lock(mtx_);
    while (!shutdown_) {
        while (!notified_) {
            thr_notify_.wait(lock);
        }

        if (nodes_ && objects_) {
            GatherDrawables(nodes_, root_node_, objects_, obj_indices_, object_count_, drawables_data_[1]);
        }

        notified_ = false;
        thr_done_.notify_one();
    }
}

void Renderer::SwapDrawLists(const Ren::Camera &cam, const bvh_node_t *nodes, uint32_t root_node, uint32_t nodes_count,
                             const SceneObject *objects, const uint32_t *obj_indices, uint32_t object_count, const Environment &env,
                             const TextureAtlas *decals_atlas) {
    using namespace RendererInternal;

    bool should_notify = false;

    if (USE_TWO_THREADS) {
        std::unique_lock<std::mutex> lock(mtx_);
        while (notified_) {
            thr_done_.wait(lock);
        }
    }

    {
        std::swap(drawables_data_[0], drawables_data_[1]);
        drawables_data_[1].render_flags = render_flags_;
        drawables_data_[1].decals_atlas = decals_atlas;
        nodes_ = nodes;
        root_node_ = root_node;
        nodes_count_ = nodes_count;
        objects_ = objects;
        obj_indices_ = obj_indices;
        object_count_ = object_count;
        drawables_data_[1].draw_cam = cam;
        drawables_data_[1].env = env;
        std::swap(depth_pixels_[0], depth_pixels_[1]);
        std::swap(depth_tiles_[0], depth_tiles_[1]);
        if (nodes != nullptr) {
            should_notify = true;
        } else {
            drawables_data_[1].draw_list.clear();
        }
    }

    if (USE_TWO_THREADS && should_notify) {
        notified_ = true;
        thr_notify_.notify_one();
    }
}

#undef BBOX_POINTS