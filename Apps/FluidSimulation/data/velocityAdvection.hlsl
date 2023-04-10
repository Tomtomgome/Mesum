#include "commonInclude.hlsl"

Texture2D<float2> velocityInput : register(t0);
RWTexture2D<float2> velocityOutput : register(u0);

float2 sample_velocity(CoordData a_uv)
{
  float2 velocity;
  velocity.x = velocityInput.SampleLevel(samplerLinearBlackBorder, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
  velocity.y = velocityInput.SampleLevel(samplerLinearBlackBorder, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
  return velocity;
}

float2 sample_velocity_x(CoordData a_uv)
{
  float2 velocity;
  velocity.x = velocityInput.SampleLevel(samplerLinear, a_uv.uv, 0).x;
  velocity.y = velocityInput.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 1, -1).uv, 0).y;
  return velocity;
}

float2 sample_velocity_y(CoordData a_uv)
{
  float2 velocity;
  velocity.x = velocityInput.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 1, 1).uv, 0).x;
  velocity.y = velocityInput.SampleLevel(samplerLinear, a_uv.uv, 0).y;
  return velocity;
}

// ---------- Advection
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_advectVelocityCellCenter(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  CoordData uv = compute_uv(DTid);

  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  // Need to separate center cell quantities advection from staggered quantities advections
  float2 velocity = sample_velocity(uv);
  //float2 velocityX = sample_velocity_x(uv);
  //float2 velocityY = sample_velocity_y(uv);

  float2 startingPoint = uv.uv - (g_time * velocity) * uv.pixel;
  uv.uv = startingPoint;
  //velocityOutput[uint2(DTid.x, DTid.y)] = sample_cubic_f2(velocityInput, samplerLinear, uv);

  //float2 startingPointX = uv.uv - (g_time * velocityX) * uv.pixel;
  //uv.uv = startingPointX;
  velocityOutput[uint2(DTid.x, DTid.y)].x = sample_cubic_f2(velocityInput, samplerLinear, uv_plusHalf(uv, 1, 0)).x;

  //float2 startingPointY = uv.uv - (g_time * velocityY) * uv.pixel;
  //uv.uv = startingPointY;
  velocityOutput[uint2(DTid.x, DTid.y)].y = sample_cubic_f2(velocityInput, samplerLinear, uv_plusHalf(uv, 0, 1)).y;
}

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_staggerVelocities(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  CoordData uv = compute_uv(DTid);
  
  if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
  {
    return;
  }

  // Stagger the velocities
  velocityOutput[uint2(DTid.x, DTid.y)].x = velocityInput.SampleLevel(samplerLinear, uv_plusHalf(uv, -1.0, 0.0).uv, 0).x;
  velocityOutput[uint2(DTid.x, DTid.y)].y = velocityInput.SampleLevel(samplerLinear, uv_plusHalf(uv, 0.0, -1.0).uv, 0).y;
  
  //velocityOutput[uint2(DTid.x, DTid.y)] = velocityInput.SampleLevel(samplerLinear, uv.uv, 0);
}