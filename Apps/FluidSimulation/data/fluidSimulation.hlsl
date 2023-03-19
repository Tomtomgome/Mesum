SamplerState basicSampler : register(s0);

Texture2D<float4> inputData : register(t0);
RWTexture2D<float4> outputData : register(u0);

static const float g_time = 0.016;

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
  float2 velocity;
  velocity.x = inputData.SampleLevel(basicSampler, uv - float2(halfPixel.x, 0.0), 0).x;
  velocity.y = inputData.SampleLevel(basicSampler, uv - float2(0.0, halfPixel.y), 0).y;
  
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

}


