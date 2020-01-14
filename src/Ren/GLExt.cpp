#define __GL_API_DEF__
#include "GL.h"
#undef __GL_API_DEF__

#include <cassert>

#if defined(__ANDROID__) || defined(__native_client__) || defined(EMSCRIPTEN)
#else
#if defined(WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <GL/glx.h>
#endif
#endif

#include "GLExtDSAEmu.h"

//#define GL_DISABLE_DSA

#undef None // defined in X.h
#undef near // defined in minwindef.h
#undef far
#undef min
#undef max

bool Ren::InitGLExtentions() {
#if defined(__ANDROID__) || defined(__native_client__) || defined(EMSCRIPTEN)
#define GetProcAddress(name) eglGetProcAddress(#name);

    glQueryCounterEXT               = (PFNGLQUERYCOUNTEREXTPROC)GetProcAddress(glQueryCounterEXT);
    glGetQueryObjecti64vEXT         = (PFNGLGETQUERYOBJECTI64VEXTPROC)GetProcAddress(glGetQueryObjecti64vEXT);
    glGetQueryObjectui64vEXT        = (PFNGLGETQUERYOBJECTUI64VEXTPROC)GetProcAddress(glGetQueryObjectui64vEXT);

    //
    // direct state access emulation
    //

    ren_glCreateTextures            = ren_glCreateTextures_emu;

    ren_glTextureStorage2D_Comp     = ren_glTextureStorage2D_Comp_emu;
    ren_glTextureStorage3D_Comp     = ren_glTextureStorage3D_Comp_emu;

    ren_glTextureSubImage2D_Comp    = ren_glTextureSubImage2D_Comp_emu;
    ren_glTextureSubImage3D_Comp    = ren_glTextureSubImage3D_Comp_emu;

    ren_glCompressedTextureSubImage2D_Comp = ren_glCompressedTextureSubImage2D_Comp_emu;
    ren_glCompressedTextureSubImage3D_Comp = ren_glCompressedTextureSubImage3D_Comp_emu;

    ren_glTextureParameterf_Comp    = ren_glTextureParameterf_Comp_emu;
    ren_glTextureParameteri_Comp    = ren_glTextureParameteri_Comp_emu;

    ren_glTextureParameterfv_Comp   = ren_glTextureParameterfv_Comp_emu;
    ren_glTextureParameteriv_Comp   = ren_glTextureParameteriv_Comp_emu;

    ren_glGenerateTextureMipmap_Comp = ren_glGenerateTextureMipmap_Comp_emu;

    ren_glBindTextureUnit_Comp      = ren_glBindTextureUnit_Comp_emu;
#else

#if defined(WIN32)
#define GetProcAddress(name) wglGetProcAddress(#name);

    if (wglGetCurrentContext() == NULL) {
        return false;
    }
#elif defined(__linux__)
#define GetProcAddress(name) glXGetProcAddress((const GLubyte *) #name);
#endif

    ren_glCreateProgram             = (PFNGLCREATEPROGRAMPROC)GetProcAddress(glCreateProgram);
    ren_glDeleteProgram             = (PFNGLDELETEPROGRAMPROC)GetProcAddress(glDeleteProgram);
    ren_glUseProgram                = (PFNGLUSEPROGRAMPROC)GetProcAddress(glUseProgram);
    ren_glAttachShader              = (PFNGLATTACHSHADERPROC)GetProcAddress(glAttachShader);
    ren_glLinkProgram               = (PFNGLLINKPROGRAMPROC)GetProcAddress(glLinkProgram);
    ren_glGetProgramiv              = (PFNGLGETPROGRAMIVPROC)GetProcAddress(glGetProgramiv);
    ren_glGetProgramInfoLog         = (PFNGLGETPROGRAMINFOLOGPROC)GetProcAddress(glGetProgramInfoLog);
    ren_glGetAttribLocation         = (PFNGLGETATTRIBLOCATIONPROC)GetProcAddress(glGetAttribLocation);
    ren_glGetUniformLocation        = (PFNGLGETUNIFORMLOCATIONPROC)GetProcAddress(glGetUniformLocation);
    ren_glGetActiveAttrib           = (PFNGLGETACTIVEATTRIBPROC)GetProcAddress(glGetActiveAttrib);
    ren_glGetActiveUniform          = (PFNGLGETACTIVEUNIFORMPROC)GetProcAddress(glGetActiveUniform);
    ren_glGetUniformBlockIndex      = (PFNGLGETUNIFORMBLOCKINDEXPROC)GetProcAddress(glGetUniformBlockIndex);
    ren_glUniformBlockBinding       = (PFNGLUNIFORMBLOCKBINDINGPROC)GetProcAddress(glUniformBlockBinding);
    ren_glVertexAttribPointer       = (PFNGLVERTEXATTRIBPOINTERPROC)GetProcAddress(glVertexAttribPointer);
    ren_glEnableVertexAttribArray   = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetProcAddress(glEnableVertexAttribArray);
    ren_glDisableVertexAttribArray  = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)GetProcAddress(glDisableVertexAttribArray);

    ren_glCreateShader              = (PFNGLCREATESHADERPROC)GetProcAddress(glCreateShader);
    ren_glDeleteShader              = (PFNGLDELETESHADERPROC)GetProcAddress(glDeleteShader);
    ren_glShaderSource              = (PFNGLSHADERSOURCEPROC)GetProcAddress(glShaderSource);
    ren_glShaderBinary              = (PFNGLSHADERBINARYPROC)GetProcAddress(glShaderBinary);
    ren_glCompileShader             = (PFNGLCOMPILESHADERPROC)GetProcAddress(glCompileShader);
    ren_glSpecializeShader          = (PFNGLSPECIALIZESHADERPROC)GetProcAddress(glSpecializeShader);
    ren_glGetShaderiv               = (PFNGLGETSHADERIVPROC)GetProcAddress(glGetShaderiv);
    ren_glGetShaderInfoLog          = (PFNGLGETSHADERINFOLOGPROC)GetProcAddress(glGetShaderInfoLog);

#if !defined(__linux__)
    ren_glActiveTexture             = (PFNGLACTIVETEXTUREPROC)GetProcAddress(glActiveTexture);
#endif
    ren_glGenerateMipmap            = (PFNGLGENERATEMIPMAPPROC)GetProcAddress(glGenerateMipmap);

    ren_glGenBuffers                = (PFNGLGENBUFFERSPROC)GetProcAddress(glGenBuffers);
    ren_glDeleteBuffers             = (PFNGLDELETEBUFFERSPROC)GetProcAddress(glDeleteBuffers);
    ren_glBindBuffer                = (PFNGLBINDBUFFERPROC)GetProcAddress(glBindBuffer);
    ren_glBufferData                = (PFNGLBUFFERDATAPROC)GetProcAddress(glBufferData);
    ren_glBufferSubData             = (PFNGLBUFFERSUBDATAPROC)GetProcAddress(glBufferSubData);
    ren_glBindBufferBase            = (PFNGLBINDBUFFERBASEPROC)GetProcAddress(glBindBufferBase);
    ren_glBindBufferRange           = (PFNGLBINDBUFFERRANGEPROC)GetProcAddress(glBindBufferRange);
    ren_glBindVertexBuffer          = (PFNGLBINDVERTEXBUFFERPROC)GetProcAddress(glBindVertexBuffer);
    ren_glCopyBufferSubData         = (PFNGLCOPYBUFFERSUBDATAPROC)GetProcAddress(glCopyBufferSubData);

    ren_glMapBuffer                 = (PFNGLMAPBUFFERPROC)GetProcAddress(glMapBuffer);
    ren_glMapBufferRange            = (PFNGLMAPBUFFERRANGEPROC)GetProcAddress(glMapBufferRange);
    ren_glFlushMappedBufferRange    = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)GetProcAddress(glFlushMappedBufferRange);
    ren_glUnmapBuffer               = (PFNGLUNMAPBUFFERPROC)GetProcAddress(glUnmapBuffer);

    ren_glGenFramebuffers           = (PFNGLGENFRAMEBUFFERSPROC)GetProcAddress(glGenFramebuffers);
    ren_glDeleteFramebuffers        = (PFNGLDELETEFRAMEBUFFERSPROC)GetProcAddress(glDeleteFramebuffers);
    ren_glBindFramebuffer           = (PFNGLBINDFRAMEBUFFERPROC)GetProcAddress(glBindFramebuffer);
    ren_glFramebufferTexture2D      = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetProcAddress(glFramebufferTexture2D);
    ren_glFramebufferTexture3D      = (PFNGLFRAMEBUFFERTEXTURE3DPROC)GetProcAddress(glFramebufferTexture3D);
    ren_glFramebufferTextureLayer   = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)GetProcAddress(glFramebufferTextureLayer);

    ren_glGenRenderbuffers          = (PFNGLGENRENDERBUFFERSPROC)GetProcAddress(glGenRenderbuffers);
    ren_glDeleteRenderbuffers       = (PFNGLDELETERENDERBUFFERSPROC)GetProcAddress(glDeleteRenderbuffers);
    ren_glBindRenderbuffer          = (PFNGLBINDRENDERBUFFERPROC)GetProcAddress(glBindRenderbuffer);
    ren_glRenderbufferStorage       = (PFNGLRENDERBUFFERSTORAGEPROC)GetProcAddress(glRenderbufferStorage);

    ren_glFramebufferRenderbuffer   = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)GetProcAddress(glFramebufferRenderbuffer);
    ren_glCheckFramebufferStatus    = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetProcAddress(glCheckFramebufferStatus);

    ren_glDrawBuffers               = (PFNGLDRAWBUFFERSPROC)GetProcAddress(glDrawBuffers);
    ren_glBindFragDataLocation      = (PFNGLBINDFRAGDATALOCATIONPROC)GetProcAddress(glBindFragDataLocation);
    
    ren_glGenVertexArrays           = (PFNGLGENVERTEXARRAYSPROC)GetProcAddress(glGenVertexArrays);
    ren_glBindVertexArray           = (PFNGLBINDVERTEXARRAYPROC)GetProcAddress(glBindVertexArray);
    ren_glDeleteVertexArrays        = (PFNGLDELETEVERTEXARRAYSPROC)GetProcAddress(glDeleteVertexArrays);

    ren_glUniform1f                 = (PFNGLUNIFORM1FPROC)GetProcAddress(glUniform1f);
    ren_glUniform2f                 = (PFNGLUNIFORM2FPROC)GetProcAddress(glUniform2f);
    ren_glUniform3f                 = (PFNGLUNIFORM3FPROC)GetProcAddress(glUniform3f);
    ren_glUniform4f                 = (PFNGLUNIFORM4FPROC)GetProcAddress(glUniform4f);

    ren_glUniform1i                 = (PFNGLUNIFORM1IPROC)GetProcAddress(glUniform1i);
    ren_glUniform2i                 = (PFNGLUNIFORM2IPROC)GetProcAddress(glUniform2i);
    ren_glUniform3i                 = (PFNGLUNIFORM3IPROC)GetProcAddress(glUniform3i);

    ren_glUniform1iv                = (PFNGLUNIFORM1IVPROC)GetProcAddress(glUniform1iv);
    ren_glUniform2iv                = (PFNGLUNIFORM2IVPROC)GetProcAddress(glUniform2iv);
    ren_glUniform3iv                = (PFNGLUNIFORM3IVPROC)GetProcAddress(glUniform3iv);
    ren_glUniform4iv                = (PFNGLUNIFORM4IVPROC)GetProcAddress(glUniform4iv);

    ren_glUniform1ui                = (PFNGLUNIFORM1UIPROC)GetProcAddress(glUniform1ui);
    ren_glUniform2ui                = (PFNGLUNIFORM2UIPROC)GetProcAddress(glUniform2ui);
    ren_glUniform3ui                = (PFNGLUNIFORM3UIPROC)GetProcAddress(glUniform3ui);
    ren_glUniform4ui                = (PFNGLUNIFORM4UIPROC)GetProcAddress(glUniform4ui);

    ren_glUniform3fv                = (PFNGLUNIFORM3FVPROC)GetProcAddress(glUniform3fv);
    ren_glUniform4fv                = (PFNGLUNIFORM4FVPROC)GetProcAddress(glUniform4fv);

    ren_glUniformMatrix4fv          = (PFNGLUNIFORMMATRIX4FVPROC)GetProcAddress(glUniformMatrix4fv);
    ren_glUniformMatrix3x4fv        = (PFNGLUNIFORMMATRIX3X4FVPROC)GetProcAddress(glUniformMatrix3x4fv);

    ren_glCompressedTexImage2D      = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)GetProcAddress(glCompressedTexImage2D);
    ren_glCompressedTexImage3D      = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)GetProcAddress(glCompressedTexImage3D);

    ren_glCompressedTexSubImage2D   = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)GetProcAddress(glCompressedTexSubImage2D);
    ren_glCompressedTexSubImage3D   = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)GetProcAddress(glCompressedTexSubImage3D);

    ren_glTexStorage2D              = (PFNGLTEXSTORAGE2DPROC)GetProcAddress(glTexStorage2D);
    ren_glTexStorage2DMultisample   = (PFNGLTEXSTORAGE2DMULTISAMPLEPROC)GetProcAddress(glTexStorage2DMultisample);
    ren_glTexImage2DMultisample     = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)GetProcAddress(glTexImage2DMultisample);
    ren_glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)GetProcAddress(glRenderbufferStorageMultisample);

    ren_glTexStorage3D              = (PFNGLTEXSTORAGE3DPROC)GetProcAddress(glTexStorage3D);

    ren_glTexSubImage3D             = (PFNGLTEXSUBIMAGE3DPROC)GetProcAddress(glTexSubImage3D);

#if !defined(__linux__)
    ren_glTexImage3D                = (PFNGLTEXIMAGE3DPROC)GetProcAddress(glTexImage3D);
#endif

    ren_glDrawElementsBaseVertex    = (PFNGLDRAWELEMENTSBASEVERTEXPROC)GetProcAddress(glDrawElementsBaseVertex);
    ren_glDrawElementsInstanced     = (PFNGLDRAWELEMENTSINSTANCEDPROC)GetProcAddress(glDrawElementsInstanced);
    ren_glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)GetProcAddress(glDrawElementsInstancedBaseVertex);

    ren_glDispatchCompute           = (PFNGLDISPATCHCOMPUTEPROC)GetProcAddress(glDispatchCompute);
    ren_glMemoryBarrier             = (PFNGLMEMORYBARRIERPROC)GetProcAddress(glMemoryBarrier);
    ren_glGetBufferSubData          = (PFNGLGETBUFFERSUBDATAPROC)GetProcAddress(glGetBufferSubData);

    ren_glTexBuffer                 = (PFNGLTEXBUFFERPROC)GetProcAddress(glTexBuffer);
    ren_glTexBufferRange            = (PFNGLTEXBUFFERRANGEPROC)GetProcAddress(glTexBufferRange);

    ren_glGenQueries                = (PFNGLGENQUERIESPROC)GetProcAddress(glGenQueries);
    ren_glDeleteQueries             = (PFNGLDELETEQUERIESPROC)GetProcAddress(glDeleteQueries);
    ren_glQueryCounter              = (PFNGLQUERYCOUNTERPROC)GetProcAddress(glQueryCounter);

    ren_glGetQueryObjectiv          = (PFNGLGETQUERYOBJECTIVPROC)GetProcAddress(glGetQueryObjectiv);
    ren_glGetQueryObjectuiv         = (PFNGLGETQUERYOBJECTUIVPROC)GetProcAddress(glGetQueryObjectuiv);
    ren_glGetQueryObjecti64v        = (PFNGLGETQUERYOBJECTI64VPROC)GetProcAddress(glGetQueryObjecti64v);
    ren_glGetQueryObjectui64v       = (PFNGLGETQUERYOBJECTUI64V)GetProcAddress(glGetQueryObjectui64v);

    ren_glGetStringi                = (PFNGLGETSTRINGIPROC)GetProcAddress(glGetStringi);

    ren_glGetInteger64v             = (PFNGLGETINTEGER64VPROC)GetProcAddress(glGetInteger64v);
    ren_glGetBooleani_v             = (PFNGLGETBOOLEANI_VPROC)GetProcAddress(glGetBooleani_v);
    ren_glGetIntegeri_v             = (PFNGLGETINTEGERI_VPROC)GetProcAddress(glGetIntegeri_v);
    ren_glGetFloati_v               = (PFNGLGETFLOATI_VPROC)GetProcAddress(glGetFloati_v);
    ren_glGetDoublei_v              = (PFNGLGETDOUBLEI_VPROC)GetProcAddress(glGetDoublei_v);
    ren_glGetInteger64i_v           = (PFNGLGETINTEGER64I_VPROC)GetProcAddress(glGetInteger64i_v);

    ren_glGetTextureLevelParameterfv = (PFNGLGETTEXTURELEVELPARAMETERFVPROC)GetProcAddress(glGetTextureLevelParameterfv);
    ren_glGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIVPROC)GetProcAddress(glGetTextureLevelParameteriv);

    ren_glGetTextureImage           = (PFNGLGETTEXTUREIMAGEPROC)GetProcAddress(glGetTextureImage);
    ren_glGetTextureSubImage        = (PFNGLGETTEXTURESUBIMAGEPROC)GetProcAddress(glGetTextureSubImage);

    ren_glGetCompressedTextureSubImage = (PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC)GetProcAddress(glGetCompressedTextureSubImage);

    ren_glDebugMessageCallback      = (PFNGLDEBUGMESSAGECALLBACKPROC)GetProcAddress(glDebugMessageCallback);
    ren_glDebugMessageInsert        = (PFNGLDEBUGMESSAGEINSERTPROC)GetProcAddress(glDebugMessageInsert);
    ren_glPushDebugGroup            = (PFNGLPUSHDEBUGGROUPPROC)GetProcAddress(glPushDebugGroup);
    ren_glPopDebugGroup             = (PFNGLPOPDEBUGGROUPPROC)GetProcAddress(glPopDebugGroup);

    ren_glObjectLabel               = (PFNGLOBJECTLABELPROC)GetProcAddress(glObjectLabel);

    ren_glFenceSync                 = (PFNGLFENCESYNCPROC)GetProcAddress(glFenceSync);
    ren_glClientWaitSync            = (PFNGLCLIENTWAITSYNCPROC)GetProcAddress(glClientWaitSync);
    ren_glDeleteSync                = (PFNGLDELETESYNCPROC)GetProcAddress(glDeleteSync);

    ren_glBlendFunci                = (PFNGLBLENDFUNCI)GetProcAddress(glBlendFunci);
    ren_glClearBufferfv             = (PFNGLCLEARBUFFERFV)GetProcAddress(glClearBufferfv);

    //
    // direct state access
    //

#ifndef GL_DISABLE_DSA
    ren_glCreateTextures            = (PFNGLCREATETEXTURESPROC)GetProcAddress(glCreateTextures);
    if (!ren_glCreateTextures) ren_glCreateTextures = ren_glCreateTextures_emu;

    ren_glTextureStorage2D          = (PFNGLTEXTURESTORAGE2DPROC)GetProcAddress(glTextureStorage2D);
    if (ren_glTextureStorage2D) {
        ren_glTextureStorage2D_Comp = [](GLenum /*target*/, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
            ren_glTextureStorage2D(texture, levels, internalformat, width, height);
        };
    } else {
        ren_glTextureStorage2D_Comp = ren_glTextureStorage2D_Comp_emu;
    }

    ren_glTextureStorage3D = (PFNGLTEXTURESTORAGE3DPROC)GetProcAddress(glTextureStorage3D);
    if (ren_glTextureStorage3D) {
        ren_glTextureStorage3D_Comp = [](GLenum /*target*/, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
            ren_glTextureStorage3D(texture, levels, internalformat, width, height, depth);
        };
    } else {
        ren_glTextureStorage3D_Comp = ren_glTextureStorage3D_Comp_emu;
    }

    ren_glTextureSubImage2D         = (PFNGLTEXTURESUBIMAGE2DPROC)GetProcAddress(glTextureSubImage2D);
    if (ren_glTextureSubImage2D) {
        ren_glTextureSubImage2D_Comp = [](
                GLenum /*target*/, GLuint texture, GLint level, GLint xoffset, GLint yoffset,
                GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
            ren_glTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, type, pixels);
        };
    } else {
        ren_glTextureSubImage2D_Comp = ren_glTextureSubImage2D_Comp_emu;
    }

    ren_glTextureSubImage3D = (PFNGLTEXTURESUBIMAGE3DPROC)GetProcAddress(glTextureSubImage3D);
    if (ren_glTextureSubImage3D) {
        ren_glTextureSubImage3D_Comp = [](
                GLenum /*target*/, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) {
            ren_glTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
        };
    } else {
        ren_glTextureSubImage3D_Comp = ren_glTextureSubImage3D_Comp_emu;
    }

    ren_glCompressedTextureSubImage2D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC)GetProcAddress(glCompressedTextureSubImage2D);
    if (ren_glCompressedTextureSubImage2D) {
        ren_glCompressedTextureSubImage2D_Comp = [](GLenum /*target*/,
                GLuint texture, GLint level, GLint xoffset, GLint yoffset,
                GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
            ren_glCompressedTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, imageSize, data);
        };
    } else {
        ren_glCompressedTextureSubImage2D_Comp = ren_glCompressedTextureSubImage2D_Comp_emu;
    }

    ren_glCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC)GetProcAddress(glCompressedTextureSubImage3D);
    if (ren_glCompressedTextureSubImage3D) {
        ren_glCompressedTextureSubImage3D_Comp = [](GLenum /*target*/,
                GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) {
            ren_glCompressedTextureSubImage3D(
                    texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
        };
    } else {
        ren_glCompressedTextureSubImage3D_Comp = ren_glCompressedTextureSubImage3D_Comp_emu;
    }

    ren_glTextureParameterf         = (PFNGLTEXTUREPARAMETERFPROC)GetProcAddress(glTextureParameterf);
    if (ren_glTextureParameterf) {
        ren_glTextureParameterf_Comp = [](GLenum /*target*/, GLuint texture, GLenum pname, GLfloat param) { ren_glTextureParameterf(texture, pname, param); };
    } else {
        ren_glTextureParameterf_Comp = ren_glTextureParameterf_Comp_emu;
    }
    ren_glTextureParameteri         = (PFNGLTEXTUREPARAMETERIPROC)GetProcAddress(glTextureParameteri);
    if (ren_glTextureParameteri) {
        ren_glTextureParameteri_Comp = [](GLenum /*target*/, GLuint texture, GLenum pname, GLint param) { ren_glTextureParameteri(texture, pname, param); };
    } else {
        ren_glTextureParameteri_Comp = ren_glTextureParameteri_Comp_emu;
    }

    ren_glTextureParameterfv        = (PFNGLTEXTUREPARAMETERFVPROC)GetProcAddress(glTextureParameterfv);
    if (ren_glTextureParameterfv) {
        ren_glTextureParameterfv_Comp = [](GLenum /*target*/, GLuint texture, GLenum pname, const GLfloat *params) {
            ren_glTextureParameterfv(texture, pname, params);
        };
    } else {
        ren_glTextureParameterfv_Comp = ren_glTextureParameterfv_Comp_emu;
    }
    ren_glTextureParameteriv        = (PFNGLTEXTUREPARAMETERIVPROC)GetProcAddress(glTextureParameteriv);
    if (ren_glTextureParameteriv) {
        ren_glTextureParameteriv_Comp = [](GLenum /*target*/, GLuint texture, GLenum pname, const GLint *params) {
            ren_glTextureParameteriv(texture, pname, params);
        };
    } else {
        ren_glTextureParameteriv_Comp = ren_glTextureParameteriv_Comp_emu;
    }

    ren_glGenerateTextureMipmap     = (PFNGLGENERATETEXTUREMIPMAPPROC)GetProcAddress(glGenerateTextureMipmap);
    if (ren_glGenerateTextureMipmap) {
        ren_glGenerateTextureMipmap_Comp = [](GLenum /*target*/, GLuint texture) { ren_glGenerateTextureMipmap(texture); };
    } else {
        ren_glGenerateTextureMipmap_Comp = ren_glGenerateTextureMipmap_Comp_emu;
    }

    ren_glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)GetProcAddress(glBindTextureUnit);
    if (ren_glBindTextureUnit) {
        ren_glBindTextureUnit_Comp = [](GLenum /*target*/, GLuint unit, GLuint texture) { ren_glBindTextureUnit(unit, texture); };
    } else {
        ren_glBindTextureUnit_Comp = ren_glBindTextureUnit_Comp_emu;
    }
#else
    ren_glCreateTextures            = ren_glCreateTextures_emu;

    ren_glTextureStorage2D_Comp     = ren_glTextureStorage2D_Comp_emu;
    ren_glTextureStorage3D_Comp     = ren_glTextureStorage3D_Comp_emu;

    ren_glTextureSubImage2D_Comp    = ren_glTextureSubImage2D_Comp_emu;
    ren_glTextureSubImage3D_Comp    = ren_glTextureSubImage3D_Comp_emu;

    ren_glCompressedTextureSubImage2D_Comp = ren_glCompressedTextureSubImage2D_Comp_emu;
    ren_glCompressedTextureSubImage3D_Comp = ren_glCompressedTextureSubImage3D_Comp_emu;

    ren_glTextureParameterf_Comp    = ren_glTextureParameterf_Comp_emu;
    ren_glTextureParameteri_Comp    = ren_glTextureParameteri_Comp_emu;

    ren_glTextureParameterfv_Comp   = ren_glTextureParameterfv_Comp_emu;
    ren_glTextureParameteriv_Comp   = ren_glTextureParameteriv_Comp_emu;

    ren_glGenerateTextureMipmap_Comp = ren_glGenerateTextureMipmap_Comp_emu;

    ren_glBindTextureUnit_Comp      = ren_glBindTextureUnit_Comp_emu;
#endif
#endif

    return true;
}