struct Vertex
{
  float4 position : POSITION0;
  float4 color : COLOR0;
  float2 uv : TEXCOORD0;
};

struct VertexShaderOutput
{
  float4 position : SV_POSITION0;
  float4 color : COLOR0;
  float2 uv : TEXCOORD0;
};

struct Vs2DMatrices
{
  matrix MVP;
};
ConstantBuffer<Vs2DMatrices> CBMatrices : register(b0);

VertexShaderOutput vs_main(Vertex a_in)
{
  VertexShaderOutput output;

  output.position = mul(CBMatrices.MVP, float4(a_in.position.xy, 0.0, 1.0));
  output.color = a_in.color;
  output.uv = a_in.uv;

  return output;
}

//---------------
//---------------

struct Pixel
{
  float4 color : SV_TARGET0;
};

struct PsTextureID
{
  int textureIndex;
};
ConstantBuffer<PsTextureID> CBMaterial : register(b1);

Texture2D<float4> textures[] : register(t0, space1);
SamplerState basicSampler : register(s0);

Pixel ps_main(VertexShaderOutput a_in)
{
  Pixel output;
  
  output.color = a_in.color * textures[CBMaterial.textureIndex].Sample(basicSampler, a_in.uv);

  return output;
}
