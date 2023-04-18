#include "commonInclude.hlsl"

Texture2D<float4> inputData : register(t0);
Texture2D<float2> inputVelocity : register(t1);
RWTexture2D<float4> outputData : register(u0);

float2 sample_velocity(CoordData a_uv)
{
  float2 velocity;
  velocity.x = inputVelocity.SampleLevel(samplerLinear, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
  velocity.y = inputVelocity.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
  return velocity;
}

// ---------- Advection
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_advect(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  CoordData uv = compute_uv(DTid);
  
  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  // Need to separate center cell quantities advection from staggered quantities advections
  float2 velocity = sample_velocity(uv);

  float2 startingPoint = uv.uv - (g_time * velocity / data.cellSize) * uv.pixel;
  uv.uv = startingPoint;

  outputData[uint2(DTid.x, DTid.y)] = sample_cubic_f4(inputData, samplerLinear, uv);
}
