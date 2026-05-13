#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inColor;
layout(location = 3) in uint inControl;

layout(location = 0) out vec2 outTextureCoordinate;
layout(location = 1) flat out uint outColor;
layout(location = 2) flat out uint outControl;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    outTextureCoordinate = inTexCoord;
    outColor = inColor;
    outControl = inControl;
}
