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
  
    if(DTid.x == 0 || DTid.y == 0 || DTid.x >= data.resolution.x -1 || DTid.y >= data.resolution.y - 1)
    //if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    float invDxSqr = 1.0/(data.cellSize.x * data.cellSize.y);

    float p_i_j = inputPressure.SampleLevel(samplerPoint, uv.uv, 0);

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

    float noWallNumber = 4;
    noWallNumber -= (data.wallAtLeft && DTid.x == 1) ? 1 : 0;
    noWallNumber -= (data.wallAtBottom && DTid.y == 1) ? 1 : 0;
    noWallNumber -= (data.wallAtRight && DTid.x == data.resolution.x-2) ? 1 : 0;
    noWallNumber -= (data.wallAtTop && DTid.y == data.resolution.y-2) ? 1 : 0;

    float ap = g_invDensity*(noWallNumber*p_i_j - (pRight + pLeft + pTop + pBottom))*invDxSqr;

    float d = inputDivergence.SampleLevel(samplerPoint, uv.uv, 0);

    outputResidual[uint2(DTid.x, DTid.y)] = d - ap;
}
