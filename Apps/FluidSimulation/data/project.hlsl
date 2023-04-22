#include "commonInclude.hlsl"

Texture2D<float> pressure : register(t0);
RWTexture2D<float2> velocityOutput : register(u0);

// ---------- projection

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_project(uint3 DTid : SV_DispatchThreadID)
{  
    CoordData uv = compute_uv(DTid);
    //if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    if(DTid.x == 0 || DTid.y == 0 || DTid.x >= data.resolution.x -1 || DTid.y >= data.resolution.y - 1)
    {
        return;
    }

    float pij = pressure.SampleLevel(samplerPoint, uv.uv, 0);
    float piPlus1j;
    if(DTid.x == data.resolution.x - 2)
    {
        piPlus1j = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 0).uv, 0);

        if(data.wallAtRight)
        {
            piPlus1j = pij + ((g_density * data.cellSize.x) * velocityOutput[uint2(DTid.x, DTid.y)].x);
        }
    }
    else
    {
        piPlus1j = pressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
    }

    float pijPlus1;
    if(DTid.y == data.resolution.y - 2)
    {
        pijPlus1 = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 0).uv, 0);
        if(data.wallAtTop)
        {
            pijPlus1 = pij + ((g_density * data.cellSize.y) * velocityOutput[uint2(DTid.x, DTid.y)].y);
        }
    }
    else
    {
        pijPlus1 = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
    }

    if(DTid.x != data.resolution.x - 2)
        velocityOutput[uint2(DTid.x, DTid.y)].x = velocityOutput[uint2(DTid.x, DTid.y)].x - (piPlus1j - pij) / (g_density * data.cellSize.x);
    if(DTid.y != data.resolution.y - 2)
        velocityOutput[uint2(DTid.x, DTid.y)].y = velocityOutput[uint2(DTid.x, DTid.y)].y - (pijPlus1 - pij) / (g_density * data.cellSize.y);
}

