SamplerState basicSampler : register(s0);

Texture2D<float4> inputData : register(t0);
RWTexture2D<float4> outputData : register(u0);

[numthreads( 1, 1, 1 )]
void cs_advect(uint3 DTid : SV_DispatchThreadID)
{
  float g_time = 0.016;
  // IMPROVE compute index 
  uint dimX, dimY;
  inputData.GetDimensions(dimX, dimY);
  float2 invDim = float2(1.0f/(dimX-1), 1.0f/(dimY-1));
  float2 uv = float2(invDim.x * DTid.x, invDim.y * DTid.y);
  float2 halfPixel = 0.5f*invDim;

  // Need to separate center cell quantities advection from staggered quantities advections
  float2 velocity;
  velocity.x = inputData.SampleLevel(basicSampler, uv - float2(halfPixel.x, 0.0), 0).r;
  velocity.y = inputData.SampleLevel(basicSampler, uv - float2(0.0, halfPixel.y), 0).g;
  
  float2 startingPoint = uv - g_time * velocity;

  //Improve with cubic interpolation
  outputData[uint2(DTid.x, DTid.y)] = inputData.SampleLevel(basicSampler, startingPoint, 0);
}


