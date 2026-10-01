#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) flat in uint in_color;
layout(location = 2) flat in uint in_control;

layout(set = 0, binding = 0) uniform sampler2D color_texture;

layout(location = 0) out vec4 out_color;

vec4 unpack_color(uint packed_color) {
    return vec4(
        float(packed_color & 0xffu),
        float((packed_color >> 8) & 0xffu),
        float((packed_color >> 16) & 0xffu),
        float((packed_color >> 24) & 0xffu)
    ) / 255.0;
}

void main() {
    vec4 texture_color = texture(color_texture, uv);

    if (texture_color.a == 0.0) {
        discard;
    }

    out_color = texture_color * unpack_color(in_color);
}
