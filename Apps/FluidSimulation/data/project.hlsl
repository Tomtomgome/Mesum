#include "commonInclude.hlsl"

Texture2D<float> pressure : register(t0);
RWTexture2D<float2> outputVelocity : register(u0);

// ---------- projection

static const float g_alphaJacob = -(g_cellSize*g_cellSize);
static const float g_betaJacob = 1.0/4.0;

static const float g_globalFactor = 1.0;

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_project(uint3 DTid : SV_DispatchThreadID)
{  
    CoordData uv = compute_uv(DTid);
    if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    float pij = pressure.SampleLevel(samplerPoint, uv.uv, 0);
    float piPlus1j;
    if(DTid.x == data.resolution.x-1)
    {
        piPlus1j = pij + ((g_density * g_cellSize) * outputVelocity[uint2(DTid.x, DTid.y)].x);
    }
    else
    {
        piPlus1j = pressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
    }

    float pijPlus1;
    if(DTid.y == data.resolution.y-1)
    {
        pijPlus1 = pij + ((g_density * g_cellSize) * outputVelocity[uint2(DTid.x, DTid.y)].y);
    }
    else
    {
        pijPlus1 = pressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
    }

    outputVelocity[uint2(DTid.x, DTid.y)].x = outputVelocity[uint2(DTid.x, DTid.y)].x - (piPlus1j - pij) / (g_density * g_cellSize);
    outputVelocity[uint2(DTid.x, DTid.y)].y = outputVelocity[uint2(DTid.x, DTid.y)].y - (pijPlus1 - pij) / (g_density * g_cellSize);
}

