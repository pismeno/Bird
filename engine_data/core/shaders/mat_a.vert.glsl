#version 450

layout(binding = 0) uniform ViewData {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} ubo;

struct Quad {
    vec2 col0;       // Scale X, Shear Y
    vec2 col1;       // Shear X, Scale Y
    vec2 col2;       // Translation (Pos X, Pos Y)
    uint materialIndex;
    uint _pad;
};

layout(std430, set = 0, binding = 1) readonly buffer QuadBuffer {
    Quad quads[];
};

layout(location = 0) out vec2 fragUV;
layout(location = 1) flat out uint fragMaterialIndex;

const vec2 UNIT_CORNERS[6] = vec2[6](
vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0)
);

void main() {
    uint quadIndex = uint(gl_VertexIndex) / 6u;
    uint cornerIndex = uint(gl_VertexIndex) % 6u;

    // Pull the data for this specific quad from the SSBO
    Quad quad = quads[quadIndex];
    vec2 unitPos = UNIT_CORNERS[cornerIndex];

    // Apply the 2D affine transformation (mat3x2 math)
    mat2 basis = mat2(quad.col0, quad.col1);
    vec2 worldPos = basis * unitPos + quad.col2;

    // Project to screen
    gl_Position = ubo.viewProj * vec4(worldPos, 0.0, 1.0);

    // Pass data to fragment shader
    fragUV = unitPos;
    fragMaterialIndex = quad.materialIndex;
}