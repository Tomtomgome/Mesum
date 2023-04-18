#define COMPUTE_GROUP_SIZE 16

struct VertexInput
{
    float4 positionCS : POSITION0;
    float4 color : COLOR0;
};

struct VertexShaderOutput
{
  float4 positionCS : SV_POSITION0;
  float4 color : COLOR0;
};

struct Pixel
{
  float4 color : SV_TARGET0;
};

//---------------
//---------------

Texture2D<float2> velocity : register(t0);
SamplerState basicSampler : register(s0);

struct Data
{
  uint2 resolution;
};
ConstantBuffer<Data> data : register(b0);

RWStructuredBuffer<VertexInput> BufferOut : register(u0);
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{
  uint dimX = data.resolution.x;
  uint dimY = data.resolution.y;

  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  float2 invDim = float2(1.0f/dimX, 1.0f/dimY);
  float2 halfPixel = 0.5f*invDim;
  float2 uv = halfPixel + float2(invDim.x * DTid.x, invDim.y * DTid.y);
  float2 color = velocity.SampleLevel(basicSampler, uv, 0);

  const uint g_nbVertexPerArrow = 4;
  const float g_arrowFinsScale = 0.01f;
  const float g_arrowScale = 0.03f * 1.0f/10.0f; // Adapt to simulation
  const float4 g_defaultColor = float4(0.5f, 0.2f, 0.3f, 0.5f);

  uint baseVextexID = g_nbVertexPerArrow * (DTid.x * dimY + DTid.y);


  float unitX = 2.0f / dimX;
  float unitY = 2.0f / dimY;
  float halfUnitX = unitX * 0.5f;
  float halfUnitY = unitY * 0.5f;
  float2 baseCoord = float2(-1.0f + halfUnitX, -1.0f + halfUnitY);

  float arrowFinsScale = g_arrowFinsScale * min(1.0, length(color.xy));

  float2 arrowOrigin    = baseCoord + float2(DTid.x * unitX, DTid.y * unitY);
  float2 arrowTop = color.xy * g_arrowScale;
  float2 arrowDirection = -normalize(arrowTop.xy);
  float arrowAngle = atan2(arrowDirection.y, arrowDirection.x);

  BufferOut[baseVextexID].positionCS = float4(arrowOrigin, 0.0, 1.0);
  BufferOut[baseVextexID].color = g_defaultColor;
  BufferOut[baseVextexID + 1].positionCS = float4(arrowOrigin + arrowTop, 0.0, 1.0);
  BufferOut[baseVextexID + 1].color = g_defaultColor;
  BufferOut[baseVextexID + 2].positionCS = float4(arrowOrigin + arrowTop + float2(cos(arrowAngle - 0.75), sin(arrowAngle - 0.75)) * arrowFinsScale, 0.0, 1.0);
  BufferOut[baseVextexID + 2].color = g_defaultColor;
  BufferOut[baseVextexID + 3].positionCS = float4(arrowOrigin + arrowTop + float2(cos(arrowAngle + 0.75), sin(arrowAngle + 0.75)) * arrowFinsScale, 0.0, 1.0);
  BufferOut[baseVextexID + 3].color = g_defaultColor;
}

//---------------
//---------------

VertexShaderOutput vs_main(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.positionCS = IN.positionCS;
    OUT.color = IN.color;

    return OUT;
}

//---------------
//---------------

Pixel ps_main(VertexShaderOutput a_in)
{
  Pixel output;
  
  output.color = a_in.color;

  return output;
}

