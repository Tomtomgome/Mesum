SamplerState basicSampler : register(s0);

Texture2D<float4> inputData : register(t0);
RWTexture2D<float4> outputData : register(u0);

static const float g_time = 0.016;

static const float g_cellSize = 1.0f;

float2 sample_velocity(float2 a_uv, float2 a_halfPixel)
{
  float2 velocity;
  velocity.x = inputData.SampleLevel(basicSampler, a_uv - float2(a_halfPixel.x, 0.0), 0).x;
  velocity.y = inputData.SampleLevel(basicSampler, a_uv - float2(0.0, a_halfPixel.y), 0).y;
  return velocity;
}

// ---------- Advection
[numthreads( 1, 1, 1 )]
void cs_advect(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  uint dimX, dimY;
  inputData.GetDimensions(dimX, dimY);
  float2 invDim = float2(1.0f/dimX, 1.0f/dimY);
  float2 halfPixel = 0.5f*invDim;
  float2 uv = halfPixel + float2(invDim.x * DTid.x, invDim.y * DTid.y);

  // Need to separate center cell quantities advection from staggered quantities advections
  float2 velocity = sample_velocity(uv, halfPixel);

  float2 startingPoint = uv - (g_time * velocity) * invDim;

  //Improve with cubic interpolation
  outputData[uint2(DTid.x, DTid.y)] = inputData.SampleLevel(basicSampler, startingPoint, 0);
}

// ---------- applyForces
static const float g_gravity           = -9.8;
static const float g_alpha             = -2.5;
static const float g_beta              = 8.2;
static const float g_vorticityStrength = 0.4;

static const float g_ambientT = 270;

float compute_vorticity(float2 a_uv, float2 a_halfPixel, float2 a_invDim)
{
  float2 speed_dx = 
    sample_velocity(a_uv + float2(a_invDim.x, 0.0), a_halfPixel) - 
    sample_velocity(a_uv - float2(a_invDim.x, 0.0), a_halfPixel);
  float dv_dx = speed_dx.y / (2 * g_cellSize);
  float2 speed_dy =
    sample_velocity(a_uv + float2(0.0, a_invDim.y), a_halfPixel) - 
    sample_velocity(a_uv - float2(0.0, a_invDim.y), a_halfPixel);
  float du_dy = speed_dy.x / (2 * g_cellSize);

  return dv_dx - du_dy;
}

float2 compute_vorticityForce(float2 a_uv, float2 a_halfPixel, float2 a_invDim)
{
  float input_iplusone = abs(compute_vorticity(a_uv + float2(a_invDim.x, 0.0), 
                                                a_halfPixel, 
                                                a_invDim));
  float input_iminusone = abs(compute_vorticity(a_uv - float2(a_invDim.x, 0.0), 
                                                a_halfPixel, 
                                                a_invDim));
  float input_jplusone = abs(compute_vorticity(a_uv + float2(0.0, a_invDim.y), 
                                                a_halfPixel, 
                                                a_invDim));
  float input_jminusone = abs(compute_vorticity(a_uv - float2(0.0, a_invDim.y), 
                                                a_halfPixel, 
                                                a_invDim));

  float input_ij = compute_vorticity(a_uv, a_halfPixel, a_invDim);

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
  uint dimX, dimY;
  inputData.GetDimensions(dimX, dimY);
  float2 invDim = float2(1.0f/dimX, 1.0f/dimY);
  float2 halfPixel = 0.5f*invDim;
  float2 uv = halfPixel + float2(invDim.x * DTid.x, invDim.y * DTid.y);

  // base copy
  outputData[uint2(DTid.x, DTid.y)] = inputData.SampleLevel(basicSampler, uv, 0);
  
  // gravity
  outputData[uint2(DTid.x, DTid.y)].y += g_time * g_gravity;

  // Boyancy
  float2 data = inputData.SampleLevel(basicSampler, uv + float2(0.0, halfPixel.y), 0).zw;
  outputData[uint2(DTid.x, DTid.y)].y += g_time * (g_alpha * data.y + g_beta * (data.x - g_ambientT));

  // Vorticity Confinment
  float2 vorticityForceX =
                    0.5 * 
                    (compute_vorticityForce(uv, halfPixel, invDim) + 
                    compute_vorticityForce(uv + float2(invDim.x, 0.0), halfPixel, invDim));
  float2 vorticityForceY =
                    0.5 *
                    (compute_vorticityForce(uv, halfPixel, invDim) + 
                    compute_vorticityForce(uv + float2(0.0, invDim.y), halfPixel, invDim));

  outputData[uint2(DTid.x, DTid.y)].x += g_time * g_vorticityStrength *
                    g_cellSize * vorticityForceX.x;
  outputData[uint2(DTid.x, DTid.y)].y += g_time * g_vorticityStrength *
                    g_cellSize * vorticityForceY.y;
}


