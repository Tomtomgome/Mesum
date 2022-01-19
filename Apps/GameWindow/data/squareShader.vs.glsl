#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUv;

layout(binding = 0, row_major) uniform Vs2DMatrices
{
  mat4 MVP;
} CBMatrices;

void main() {
    gl_Position = CBMatrices.MVP * vec4(inPosition.xy, 0.0, 1.0);
    outColor = inColor;
    outUv = inUv;
}