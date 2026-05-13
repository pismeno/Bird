#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) flat in uint inColor;
layout(location = 2) flat in uint inControl;

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

layout(location = 0) out vec4 outColor;

vec4 unpack_color(uint packedColor) {
    return vec4(
        float(packedColor & 0xffu),
        float((packedColor >> 8) & 0xffu),
        float((packedColor >> 16) & 0xffu),
        float((packedColor >> 24) & 0xffu)
    ) / 255.0;
}

void main() {
    vec4 colorSample = texture(colorTexture, uv);
    
    if (colorSample.a < 0.5) {
        discard;
    }
    
    outColor = colorSample * unpack_color(inColor);
}
