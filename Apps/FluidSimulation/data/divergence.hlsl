#include "commonInclude.hlsl"

Texture2D<float2> inputVelocity : register(t0);
RWTexture2D<float> divergence : register(u0);

static const float g_globalFactor = 1.0;

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_divergence(uint3 DTid : SV_DispatchThreadID)
{  
    CoordData uv = compute_uv(DTid);
    if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }
  
    float2 iplus12 = inputVelocity.SampleLevel(samplerPointBlackBorder, uv.uv, 0).xy;
    float2 iminus12;
    iminus12.x = inputVelocity.SampleLevel(samplerPointBlackBorder, uv_plus(uv, -1, 0).uv, 0).x;
    iminus12.y = inputVelocity.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, -1).uv, 0).y;

    divergence[uint2(DTid.x, DTid.y)] = g_globalFactor * ((iplus12.x - iminus12.x) / g_cellSize +
                                (iplus12.y - iminus12.y) / g_cellSize);
}