#include "commonInclude.hlsl"

Texture2D<float4> inputData : register(t0);
RWTexture2D<float4> outputData : register(u0);


CoordData compute_uv(uint3 a_DTid)
{
  CoordData uv;
  uint dimX, dimY;
  inputData.GetDimensions(dimX, dimY);
  uv.pixel = float2(1.0f/dimX, 1.0f/dimY);
  uv.halfPixel = 0.5f*uv.pixel;
  uv.uv = uv.halfPixel + float2(uv.pixel.x * a_DTid.x, uv.pixel.y * a_DTid.y);
  return uv;
}

float2 sample_velocity(CoordData a_uv)
{
  float2 velocity;
  velocity.x = inputData.SampleLevel(samplerLinear, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
  velocity.y = inputData.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
  return velocity;
}

// ---------- Advection
[numthreads( 1, 1, 1 )]
void cs_advect(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  CoordData uv = compute_uv(DTid);

  // Need to separate center cell quantities advection from staggered quantities advections
  float2 velocity = sample_velocity(uv);

  float2 startingPoint = uv.uv - (g_time * velocity) * uv.pixel;
  uv.uv = startingPoint;

  //Improve with cubic interpolation
  outputData[uint2(DTid.x, DTid.y)] = sample_cubic_f4(inputData, samplerLinear, uv);// inputData.SampleLevel(samplerLinear, startingPoint, 0);
}

// ---------- applyForces
static const float g_gravity           = -9.8;
static const float g_alpha             = -2;
static const float g_beta              = 6.5;
static const float g_vorticityStrength = 0.8;

static const float g_ambientT = 270;

float compute_vorticity(CoordData a_uv)
{
  float2 speed_dx = 
    sample_velocity(uv_plus(a_uv, 1, 0)) - 
    sample_velocity(uv_plus(a_uv, -1, 0));
  float dv_dx = speed_dx.y / (2 * g_cellSize);
  float2 speed_dy =
    sample_velocity(uv_plus(a_uv, 0, 1)) - 
    sample_velocity(uv_plus(a_uv, 0, -1));
  float du_dy = speed_dy.x / (2 * g_cellSize);

  return dv_dx - du_dy;
}

float2 compute_vorticityForce(CoordData a_uv)
{
  float input_iplusone = abs(compute_vorticity(uv_plus(a_uv, 1, 0)));
  float input_iminusone = abs(compute_vorticity(uv_plus(a_uv, -1, 0)));
  float input_jplusone = abs(compute_vorticity(uv_plus(a_uv, 0, 1)));
  float input_jminusone = abs(compute_vorticity(uv_plus(a_uv, 0, -1)));

  float input_ij = compute_vorticity(a_uv);

  float2 outGradient;
  outGradient.x = (input_iplusone - input_iminusone) / (2 * g_cellSize);
  outGradient.y = (input_jplusone - input_jminusone) / (2 * g_cellSize);

  float gradientSqLen = dot(outGradient, outGradient);
  float len = max(sqrt(gradientSqLen), 0.0000001);
  outGradient = outGradient / len;
  outGradient = float2(outGradient.y * input_ij, outGradient.x * input_ij);

  return outGradient;
}

[numthreads( 1, 1, 1 )]
void cs_simulation(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  CoordData uv = compute_uv(DTid);

  // base copy
  outputData[uint2(DTid.x, DTid.y)] = inputData.SampleLevel(samplerPoint, uv.uv, 0);
  
  // gravity
  //outputData[uint2(DTid.x, DTid.y)].y += g_time * g_gravity;

  // Boyancy
  float2 data = inputData.SampleLevel(samplerLinear, uv_plusHalf(uv, 0, 1).uv, 0).zw;
  outputData[uint2(DTid.x, DTid.y)].y += g_time * (g_alpha * data.y + g_beta * (data.x - g_ambientT));

  // Vorticity Confinment
  float2 vorticityForceX =
                    0.5 * 
                    (compute_vorticityForce(uv) + 
                    compute_vorticityForce(uv_plus(uv, 1, 0)));
  float2 vorticityForceY =
                    0.5 *
                    (compute_vorticityForce(uv) + 
                    compute_vorticityForce(uv_plus(uv, 0, 1)));

  outputData[uint2(DTid.x, DTid.y)].x += g_time * g_vorticityStrength * g_cellSize * vorticityForceX.x;
  outputData[uint2(DTid.x, DTid.y)].y += g_time * g_vorticityStrength * g_cellSize * vorticityForceY.y;
}

