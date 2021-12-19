#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#pragma shader_stage(fragment)

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(binding = 1) uniform PsTextureID
{
  int textureIndex;
} CBMaterial;

void main() {
    outColor = inColor * texture(textures[CBMaterial.textureIndex], inUv);
}