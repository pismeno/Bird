#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 textureCoordinate;

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 colorSample = texture(colorTexture, textureCoordinate);
    
    if (colorSample.a < 0.5) {
        discard;
    }
    
    outColor = colorSample;
}
