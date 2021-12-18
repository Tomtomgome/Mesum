#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
}