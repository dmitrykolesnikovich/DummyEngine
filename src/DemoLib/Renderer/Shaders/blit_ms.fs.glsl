R"(
#version 310 es
#extension GL_ARB_texture_multisample : enable

#ifdef GL_ES
    precision mediump float;
#endif

/*
UNIFORMS
    s_texture : 3
    multiplier : 4
    uTexSize : 5
*/
        
layout(location = 14) uniform mediump sampler2DMS s_texture;
uniform float multiplier;

in vec2 aVertexUVs_;

out vec4 outColor;

void main() {
    outColor = vec4(multiplier, multiplier, multiplier, 1.0) * texelFetch(s_texture, ivec2(aVertexUVs_), 0);
}
)"