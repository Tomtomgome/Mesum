SamplerState basicSampler : register(s0);

Texture2D<float4> inputData : register(t0);
RWTexture2D<float4> outputData : register(t0);

[numthreads( 1, 1, 1 )]
void cs_advect(uint3 DTid : SV_DispatchThreadID)
{
  // IMPROVE compute index 
  uint dimX, dimY;
  texture.GetDimensions(dimX, dimY);
  float2 invDim = float2(1.0f/dimX, 1.0f/dimY);
  float2 uv = float2(invDim.x * DTid.x, invDim.y * DTid.y);
  float2 halfPixel = 0.5*invDim;

  // Need to separate center cell quantities advection from staggered quantities advections
  float2 velocity;
  velocity.x = a_inputData.SampleLevel(basicSampler, uv - float2(a_halfPixel.x, 0.0), 0).r;
  velocity.y = a_inputData.SampleLevel(basicSampler, uv - float2(0.0, a_halfPixel.y), 0).g;
  
  float2 startingPoint = uv - g_time * velocity;

  //Improve with cubic interpolation
  outputData[uv] = a_inputData.SampleLevel(basicSampler, uv, 0);
}


