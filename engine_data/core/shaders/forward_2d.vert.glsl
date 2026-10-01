#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex_coord;
layout(location = 2) in uint in_color;
layout(location = 3) in uint in_control;

layout(set = 0, binding = 1) uniform TransformBuffer {
    vec2 local_position;
    vec2 local_scale;
    float local_rotation;
    vec3 padding; // Padding to align to 16 bytes
} transform;

layout(location = 0) out vec2 out_texture_coordinate;
layout(location = 1) flat out uint out_color;
layout(location = 2) flat out uint out_control;

void main() {
    vec2 scaled_position = in_position * transform.local_scale;
    float rotation = radians(transform.local_rotation);
    float cosRotation = cos(rotation);
    float sinRotation = sin(rotation);

    vec2 rotatedPosition = vec2(
        scaled_position.x * cosRotation - scaled_position.y * sinRotation,
        scaled_position.x * sinRotation + scaled_position.y * cosRotation
    );

    vec2 worldPosition = rotatedPosition + transform.local_position;
    gl_Position = vec4(worldPosition, 0.0, 1.0);
    out_texture_coordinate = in_tex_coord;
    out_color = in_color;
    out_control = in_control;
}


