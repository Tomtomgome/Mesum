#include "commonInclude.hlsl"

Texture2D<float> inputDivergence : register(t0);
RWTexture2D<float> pressure : register(u0);
RWTexture2D<float> nextPressure : register(u1);

// ---------- projection

static const float g_alphaJacob = -(g_cellSize*g_cellSize);
static const float g_betaJacob = 1.0/4.0;

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_iterateJacobi(uint3 DTid : SV_DispatchThreadID)
{  
  CoordData uv = compute_uv(DTid);
  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  float pLeft = pressure[uint2(max(DTid.x - 1, 0), DTid.y)];// pressure.SampleLevel(samplerPoint, uv_plus(uv, -1, 0).uv, 0);
  float pRight = pressure[uint2(min(DTid.x + 1, data.resolution.x), DTid.y)];// pressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
  float pTop = pressure[uint2(DTid.x, min(DTid.y + 1, data.resolution.y))];// pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
  float pBottom = pressure[uint2(DTid.x, max(DTid.y - 1, 0))];// pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, -1).uv, 0);

  float divergence = inputDivergence.SampleLevel(samplerPoint, uv.uv, 0).x;

  nextPressure[uint2(DTid.x, DTid.y)] = (pLeft + pRight + pTop + pBottom + g_alphaJacob * divergence) * g_betaJacob;
}


