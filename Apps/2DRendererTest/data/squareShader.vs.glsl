#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = inPosition;
    outColor = inColor;
}