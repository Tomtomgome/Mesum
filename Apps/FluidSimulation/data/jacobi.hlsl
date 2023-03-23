#include "commonInclude.hlsl"

Texture2D<float2> inputVelocity : register(t0);
Texture2D<float> pressure : register(t1);
RWTexture2D<float> nextPressure : register(u0);

CoordData compute_uv(uint3 a_DTid)
{
  CoordData uv;
  uint dimX, dimY;
  inputVelocity.GetDimensions(dimX, dimY);
  uv.pixel = float2(1.0f/dimX, 1.0f/dimY);
  uv.halfPixel = 0.5f*uv.pixel;
  uv.uv = uv.halfPixel + float2(uv.pixel.x * a_DTid.x, uv.pixel.y * a_DTid.y);
  return uv;
}

float2 sample_velocity(CoordData a_uv)
{
  float2 velocity;
  velocity.x = inputVelocity.SampleLevel(samplerLinear, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
  velocity.y = inputVelocity.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
  return velocity;
}

// ---------- projection

static const float g_alphaJacob = -(g_cellSize*g_cellSize);
static const float g_betaJacob = 1.0/4.0;

static const float g_globalFactor = 1.0;

float compute_divergence(CoordData a_uv)
{
  float2 iplus12 = inputVelocity.SampleLevel(samplerPointBlackBorder, a_uv.uv, 0).xy;
  float2 iminus12;
  iminus12.x = inputVelocity.SampleLevel(samplerPointBlackBorder, uv_plus(a_uv, -1, 0).uv, 0).x;
  iminus12.y = inputVelocity.SampleLevel(samplerPointBlackBorder, uv_plus(a_uv, 0, -1).uv, 0).y;

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


