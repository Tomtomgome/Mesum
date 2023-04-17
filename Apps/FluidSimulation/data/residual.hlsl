#include "commonInclude.hlsl"

Texture2D<float> inputDivergence : register(t0);
Texture2D<float> inputPressure : register(t1);
RWTexture2D<float4> outputResidual : register(u0);

static const float g_invDxSqr = 1;
static const float g_invDensity = 1;

// ---------- Residual
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_residual(uint3 DTid : SV_DispatchThreadID)
{
    // IMPROVE compute index
    CoordData uv = compute_uv(DTid);
  
    if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    float invDxSqr = 1.0/(data.cellSize.x * data.cellSize.y);

    float p_i_j = inputPressure.SampleLevel(samplerPoint, uv.uv, 0);
    float p_ip1_j;
    if(data.wallAtRight)
    {
        p_ip1_j = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 1, 0).uv, 0);
    }
    else
    {
        p_ip1_j = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
    }

    float p_im1_j;
    if(data.wallAtLeft)
    {
        p_im1_j = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, -1, 0).uv, 0);
    }
    else
    {
        p_im1_j = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, -1, 0).uv, 0);
    }

    float p_i_jp1;
    if(data.wallAtTop)
    {
        p_i_jp1 = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, 1).uv, 0);
    }
    else
    {
        p_i_jp1 = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
    }

    float p_i_jm1;
    if(data.wallAtBottom)
    {
        p_i_jm1 = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, -1).uv, 0);
    }
    else
    {
        p_i_jm1 = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, -1).uv, 0);
    }

    float noWallNumber = 4;
    noWallNumber -= (data.wallAtLeft && DTid.x == 0) ? 1 : 0;
    noWallNumber -= (data.wallAtBottom && DTid.y == 0) ? 1 : 0;
    noWallNumber -= (data.wallAtRight && DTid.x == data.resolution.x-1) ? 1 : 0;
    noWallNumber -= (data.wallAtTop && DTid.y == data.resolution.y-1) ? 1 : 0;

    float ap = g_invDensity*(noWallNumber*p_i_j - (p_ip1_j + p_im1_j + p_i_jp1 + p_i_jm1))*invDxSqr;

    float d = inputDivergence.SampleLevel(samplerPoint, uv.uv, 0);

    outputResidual[uint2(DTid.x, DTid.y)] = d - ap;
}
