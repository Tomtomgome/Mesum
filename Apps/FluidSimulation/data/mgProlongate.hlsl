#include "commonInclude.hlsl"

Texture2D<float> inputPressure : register(t0);
RWTexture2D<float> outputPressure : register(u0);

// ---------- Residual
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_prolongate(uint3 DTid : SV_DispatchThreadID)
{
    // IMPROVE compute index
    CoordData uv = compute_uv(DTid);
  
    if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    float prolongedPressure = inputPressure.SampleLevel(samplerLinear, uv.uv, 0);

    outputPressure[uint2(DTid.x, DTid.y)] = outputPressure[uint2(DTid.x, DTid.y)] + prolongedPressure;
}
