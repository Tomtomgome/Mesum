#include "commonInclude.hlsl"

Texture2D<float> inputDivergence : register(t0);
Texture2D<float> pressure : register(t1);
RWTexture2D<float> nextPressure : register(u0);

// ---------- projection

static const float g_alphaJacob = -(g_cellSize*g_cellSize);

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_iterateJacobi(uint3 DTid : SV_DispatchThreadID)
{  
  CoordData uv = compute_uv(DTid);
  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  float pLeft = pressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, -1, 0).uv, 0);
  float pRight = pressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 1, 0).uv, 0);
  float pTop = pressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, 1).uv, 0);
  float pBottom = pressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, -1).uv, 0);

  float divergence = inputDivergence.SampleLevel(samplerPoint, uv.uv, 0).x;

  float solidWallNumber = 4;
  solidWallNumber -= DTid.x == 0 ? 1 : 0;
  solidWallNumber -= DTid.y == 0 ? 1 : 0;
  solidWallNumber -= DTid.x == data.resolution.x-1 ? 1 : 0;
  solidWallNumber -= DTid.y == data.resolution.y-1 ? 1 : 0;

  float betaJacobi = 1.0/solidWallNumber; // Solid wall number should not be zero.

  nextPressure[uint2(DTid.x, DTid.y)] = (pLeft + pRight + pTop + pBottom + g_alphaJacob * divergence) * betaJacobi;
}


