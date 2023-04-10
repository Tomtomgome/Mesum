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

    float p_i_j = inputPressure.SampleLevel(samplerPointBlackBorder, uv.uv, 0);
    float p_ip1_j = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 1, 0).uv, 0);
    float p_im1_j = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, -1, 0).uv, 0);
    float p_i_jp1 = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, 1).uv, 0);
    float p_i_jm1 = inputPressure.SampleLevel(samplerPointBlackBorder, uv_plus(uv, 0, -1).uv, 0);


    float solidWallNumber = 0;
    solidWallNumber += DTid.x == 0 ? 1 : 0;
    solidWallNumber += DTid.y == 0 ? 1 : 0;
    solidWallNumber += DTid.x == data.resolution.x-1 ? 1 : 0;
    solidWallNumber += DTid.y == data.resolution.y-1 ? 1 : 0;

    float ap = g_invDensity*((4-solidWallNumber)*p_i_j - (p_ip1_j + p_im1_j + p_i_jp1 + p_i_jm1))*g_invDxSqr;

    float d = inputDivergence.SampleLevel(samplerPoint, uv.uv, 0);

    outputResidual[uint2(DTid.x, DTid.y)] = (ap + d);
}
