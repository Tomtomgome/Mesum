﻿struct VertexShaderOutput
{
  float4 position : SV_POSITION0;
  float2 uv : TEXCOORD0;
};

VertexShaderOutput vs_main(in uint a_VertID : SV_VertexID)
{
  VertexShaderOutput output;

  output.uv = float2(uint2(a_VertID, a_VertID << 1) & 2);;
  output.position = float4(lerp(float2(-1, 1), float2(1, -1), output.uv), 0, 1);

  return output;
}

//---------------
//---------------

struct Pixel
{
  float4 color : SV_TARGET0;
};

Texture2D<float4> textures : register(t0);
SamplerState linearSampler : register(s0);
SamplerState pointSampler : register(s1);

Pixel ps_main(VertexShaderOutput a_in)
{
  Pixel output;
  
  float4 data = textures.Sample(linearSampler, float2(a_in.uv.x, 1.0 - a_in.uv.y));
  output.color.r = data.r*100;
  output.color.g = data.g*100;
  output.color.b = data.b*100;
  output.color.a = 0.9;

  output.color.r = data.g*100;
  output.color.g = data.g*100;
  output.color.b = data.g*100;
  output.color.a = 0.9;
  return output;
}
