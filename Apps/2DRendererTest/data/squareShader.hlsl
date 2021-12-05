struct Vertex
{
  float4 position : POSITION0;
  float4 color : COLOR0;
};

struct VertexShaderOutput
{
  float4 position : SV_POSITION0;
  float4 color : COLOR0;
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

  return output;
}

struct Pixel
{
  float4 color : SV_TARGET0;
};

Pixel ps_main(VertexShaderOutput a_in)
{
  Pixel output;
  
  output.color = a_in.color;

  return output;
}
