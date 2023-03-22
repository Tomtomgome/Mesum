#include "commonInclude.hlsl"

Texture2D<float4> inputData : register(t0);
Texture2D<float> pressure : register(t1);
RWTexture2D<float> nextPressure : register(u0);

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
  inputData.GetDimensions(dimX, dimY);
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

float2 sample_velocity(CoordData a_uv)
{
  float2 velocity;
  velocity.x = inputData.SampleLevel(samplerLinear, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
  velocity.y = inputData.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
  return velocity;
}

// ---------- projection

static const float g_alphaJacob = -(g_cellSize*g_cellSize);
static const float g_betaJacob = 1.0/4.0;

static const float g_globalFactor = 1.0;

float compute_divergence(CoordData a_uv)
{
  float2 iplus12 = inputData.SampleLevel(samplerPointBlackBorder, a_uv.uv, 0).xy;
  float2 iminus12;
  iminus12.x = inputData.SampleLevel(samplerPointBlackBorder, uv_plus(a_uv, -1, 0).uv, 0).x;
  iminus12.y = inputData.SampleLevel(samplerPointBlackBorder, uv_plus(a_uv, 0, -1).uv, 0).y;

  

  return g_globalFactor * ((iplus12.x - iminus12.x) / g_cellSize +
                                (iplus12.y - iminus12.y) / g_cellSize);
}

[numthreads( 1, 1, 1 )]
void cs_iterateJacobi(uint3 DTid : SV_DispatchThreadID)
{  
  CoordData uv = compute_uv(DTid);

  float pLeft = pressure.SampleLevel(samplerPoint, uv_plus(uv, -1, 0).uv, 0);
  float pRight = pressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
  float pTop = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
  float pBottom = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, -1).uv, 0);

  float divergence = compute_divergence(uv);

  nextPressure[uint2(DTid.x, DTid.y)] = (pLeft + pRight + pTop + pBottom + g_alphaJacob * divergence) * g_betaJacob;
}


