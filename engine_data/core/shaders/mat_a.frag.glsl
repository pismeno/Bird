#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragUV;
layout(location = 1) flat in uint fragMaterialIndex;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textures[];

struct Material {
    uint test_texture;
};

layout(std430, set = 1, binding = 1) readonly buffer MaterialBuffer {
    Material materials[];
};

void main() {
    Material material = materials[fragMaterialIndex];
    vec4 texColor = texture(textures[nonuniformEXT(material.test_texture)], fragUV);
    outColor = texColor;
}