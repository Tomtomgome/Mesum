#include "commonInclude.hlsl"

Texture2D<float> inputDivergence : register(t0);
Texture2D<float> inputPressure : register(t1);
RWTexture2D<float> nextPressure : register(u0);

// ---------- projection

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_iterateJacobi(uint3 DTid : SV_DispatchThreadID)
{  
    CoordData uv = compute_uv(DTid);
    if(DTid.x == 0 || DTid.y == 0 || DTid.x >= data.resolution.x -1 || DTid.y >= data.resolution.y - 1)
    //if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    float dxSqr = (data.cellSize.x * data.cellSize.y) * g_density;

    float pBottom;
    if(DTid.y == 1)
    {
        if(data.wallAtBottom)
        {
            pBottom = 0;
        }
        else
        {
            pBottom = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 0).uv, 0);
        }
    }
    else
    {
            pBottom = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, -1).uv, 0);
    }

    float pLeft;
    if(DTid.x == 1)
    {
        if(data.wallAtLeft)
        {
            pLeft = 0;
        }
        else
        {
            pLeft = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 0).uv, 0);
        }
    }
    else
    {
            pLeft = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, -1, 0).uv, 0);
    }

    float pRight;
    if(DTid.x == data.resolution.x-2)
    {
        if(data.wallAtRight)
        {
            pRight = 0;
        }
        else
        {
            pRight = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 0).uv, 0);
        }
    }
    else
    {
            pRight = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 1, 0).uv, 0);
    }

    float pTop;
    if(DTid.y == data.resolution.y -2)
    {
        if(data.wallAtTop)
        {
            pTop = 0;
        }
        else
        {
            pTop = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 0).uv, 0);
        }
    }
    else
    {
            pTop = inputPressure.SampleLevel(samplerPoint, uv_plus(uv, 0, 1).uv, 0);
    }

    float divergence = inputDivergence.SampleLevel(samplerPoint, uv.uv, 0).x;

    float noWallNumber = 4;
    noWallNumber -= (data.wallAtLeft && DTid.x == 1) ? 1 : 0;
    noWallNumber -= (data.wallAtBottom && DTid.y == 1) ? 1 : 0;
    noWallNumber -= (data.wallAtRight && DTid.x == data.resolution.x-2) ? 1 : 0;
    noWallNumber -= (data.wallAtTop && DTid.y == data.resolution.y-2) ? 1 : 0;

    float betaJacobi = 1.0/noWallNumber; // Solid wall number should not be zero.

    nextPressure[uint2(DTid.x, DTid.y)] = (pLeft + pRight + pTop + pBottom + dxSqr * divergence) * betaJacobi;
}


