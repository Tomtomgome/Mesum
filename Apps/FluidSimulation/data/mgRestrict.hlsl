#include "commonInclude.hlsl"

Texture2D<float> inputResidual : register(t0);
RWTexture2D<float> outputResidual : register(u0);

// ---------- Residual
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_restrict(uint3 DTid : SV_DispatchThreadID)
{
    // IMPROVE compute index
    CoordData uv = compute_uv(DTid);
  
    if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    // This should properly interpolate groups of 4 pixels and put the result in the smaller sized output texture
    float residual = inputResidual.SampleLevel(samplerLinear, uv.uv, 0);
    outputResidual[uint2(DTid.x, DTid.y)] = residual;
}
