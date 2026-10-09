#version 450

layout(binding = 0) uniform ViewData {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} ubo;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in uint inTexIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) flat out uint fragTexIndex;

void main() {
    gl_Position = ubo.viewProj * vec4(inPos, 0.0, 1.0);
    fragColor = inColor;
    fragUV = inUV;
    fragTexIndex = inTexIndex;
}