#include "commonInclude.hlsl"

Texture2D<float> inputDivergence : register(t0);
Texture2D<float> inputPressure : register(t1);
RWTexture2D<float> nextPressure : register(u0);

// ---------- projection

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_iterateJacobi(uint3 DTid : SV_DispatchThreadID)
{  
  CoordData uv = compute_uv(DTid);
  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  float dxSqr = (data.cellSize.x * data.cellSize.y) * g_density;

    float pRight;
    if(data.wallAtRight)
    {
        pRight = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 1, 0).uv, 0);
    }
    else
    {
        pRight = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
    }

    float pLeft;
    if(data.wallAtLeft)
    {
        pLeft = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, -1, 0).uv, 0);
    }
    else
    {
        pLeft = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, -1, 0).uv, 0);
    }

    float pTop;
    if(data.wallAtTop)
    {
        pTop = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, 1).uv, 0);
    }
    else
    {
        pTop = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
    }

    float pBottom;
    if(data.wallAtBottom)
    {
        pBottom = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, -1).uv, 0);
    }
    else
    {
        pBottom = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, -1).uv, 0);
    }

  float divergence = inputDivergence.SampleLevel(samplerPointBlackBorder, uv.uv, 0).x;

  float noWallNumber = 4;
  noWallNumber -= (data.wallAtLeft && DTid.x == 0) ? 1 : 0;
  noWallNumber -= (data.wallAtBottom && DTid.y == 0) ? 1 : 0;
  noWallNumber -= (data.wallAtRight && DTid.x == data.resolution.x-1) ? 1 : 0;
  noWallNumber -= (data.wallAtTop && DTid.y == data.resolution.y-1) ? 1 : 0;

  float betaJacobi = 1.0/noWallNumber; // Solid wall number should not be zero.

  nextPressure[uint2(DTid.x, DTid.y)] = (pLeft + pRight + pTop + pBottom + dxSqr * divergence) * betaJacobi;
}


