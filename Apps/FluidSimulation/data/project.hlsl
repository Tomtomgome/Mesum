#include "commonInclude.hlsl"

Texture2D<float> pressure : register(t0);
RWTexture2D<float4> outputData : register(u0);

static const float g_time = 0.016;

static const float g_cellSize = 1.0f;

static const float g_density = 1.0f;

struct CoordData
{
  float2 uv;
  float2 pixel;
  float2 halfPixel;
};

CoordData compute_uv(uint3 a_DTid)
{
  CoordData uv;
  uint dimX, dimY;
  pressure.GetDimensions(dimX, dimY);
  uv.pixel = float2(1.0f/dimX, 1.0f/dimY);
  uv.halfPixel = 0.5f*uv.pixel;
  uv.uv = uv.halfPixel + float2(uv.pixel.x * a_DTid.x, uv.pixel.y * a_DTid.y);
  return uv;
}

CoordData uv_plusHalf(CoordData a_uv, int a_nbHalfX, int a_nbHalfY)
{
  CoordData result = a_uv;
  result.uv += float2(a_nbHalfX * a_uv.halfPixel.x, a_nbHalfY * a_uv.halfPixel.y);
  return result;
}

CoordData uv_plus(CoordData a_uv, int a_nbX, int a_nbY)
{
  CoordData result = a_uv;
  result.uv += float2(a_nbX * a_uv.pixel.x, a_nbY * a_uv.pixel.y);
  return result;
}

// ---------- projection

static const float g_alphaJacob = -(g_cellSize*g_cellSize);
static const float g_betaJacob = 1.0/4.0;

static const float g_globalFactor = 1.0;

[numthreads( 1, 1, 1 )]
void cs_project(uint3 DTid : SV_DispatchThreadID)
{  
  CoordData uv = compute_uv(DTid);

  float pij = pressure.SampleLevel(samplerPoint, uv.uv, 0);
  float piPlus1j = pressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
  float pijPlus1 = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
  
  outputData[uint2(DTid.x, DTid.y)].x = outputData[uint2(DTid.x, DTid.y)].x
    - g_time * (piPlus1j - pij) / (g_density * g_cellSize);
  outputData[uint2(DTid.x, DTid.y)].y = outputData[uint2(DTid.x, DTid.y)].y
    - g_time * (pijPlus1 - pij) / (g_density * g_cellSize);
}

