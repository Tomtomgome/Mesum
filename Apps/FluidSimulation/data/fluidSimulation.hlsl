#include "commonInclude.hlsl"

Texture2D<float4> inputData : register(t0);
RWTexture2D<float4> outputData : register(u0);

struct CoordData
{
  float2 uv;
  float2 pixel;
  float2 halfPixel;
};

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

CoordData uv_plusHalf(CoordData a_uv, int a_nbHalfX, int a_nbHalfY)
{
  CoordData result = a_uv;
  result.uv += float2(a_nbHalfX * a_uv.halfPixel.x, a_nbHalfY * a_uv.halfPixel.y);
  return result;
}

CoordData uv_plus(CoordData a_uv, int a_nbX, int a_nbY)
{
  CoordData result = a_uv;
  result.uv += float2(a_nbX * a_uv.pixel.x, a_nbY * a_uv.pixel.y);
  return result;
}

float2 sample_velocity(CoordData a_uv)
{
  float2 velocity;
  velocity.x = inputData.SampleLevel(samplerLinear, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
  velocity.y = inputData.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
  return velocity;
}

void restrict(inout float4 a_p, in float4 a_delta)
{
    a_p.x = (a_p.x * a_delta.x) > 0.0 ? a_p.x : 0.0;
    a_p.y = (a_p.y * a_delta.y) > 0.0 ? a_p.y : 0.0;
    a_p.z  = (a_p.z * a_delta.z) > 0.0 ? a_p.z : 0.0;
    a_p.w  = (a_p.w * a_delta.w) > 0.0 ? a_p.w : 0.0;
}

float4 interpolate_cubic(float4 a_im1, float4 a_i, float4 a_ip1, float4 a_ip2, float a_alpha)
{
    float4 di        = 0.5 * (a_ip1 - a_im1);
    float4 diPlusOne = 0.5 * (a_ip2 - a_i);

    float4 delta = (a_ip1 - a_i);

    restrict(di, delta);
    restrict(diPlusOne, delta);

    float4 output =
        a_i + a_alpha * di +
        a_alpha * a_alpha * (3.0 * delta - 2.0 * di - diPlusOne) +
        a_alpha * a_alpha * a_alpha * (-2.0 * delta + di + diPlusOne);

    return output;
}

float4 sample_cubic(in Texture2D<float4> a_tex, in SamplerState a_linearSampler, in CoordData a_uv)// in float2 uv, in float2 texSize)
{
  // We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
  // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
  // location [1, 1] in the grid, where [0, 0] is the top left corner.
    float2 samplePos = a_uv.uv / a_uv.pixel;// texSize;
    float2 texPosij = floor(samplePos - 0.5f) + 0.5f;

    float2 ratio = saturate(samplePos - texPosij);

    CoordData uv_ij = a_uv;
    uv_ij.uv = texPosij * a_uv.pixel;

    float4 im1jm1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, -1).uv, 0.0f);
    float4 ijm1   = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, -1).uv, 0.0f);
    float4 ip1jm1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, -1).uv, 0.0f);
    float4 ip2jm1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, -1).uv, 0.0f);

    float4 im1j = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, 0).uv, 0.0f);
    float4 ij = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, 0).uv, 0.0f);
    float4 ip1j = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, 0).uv, 0.0f);
    float4 ip2j = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, 0).uv, 0.0f);

    float4 im1jp1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, 1).uv, 0.0f);
    float4 ijp1   = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, 1).uv, 0.0f);
    float4 ip1jp1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, 1).uv, 0.0f);
    float4 ip2jp1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, 1).uv, 0.0f);

    float4 im1jp2 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, 2).uv, 0.0f);
    float4 ijp2   = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, 2).uv, 0.0f);
    float4 ip1jp2 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, 2).uv, 0.0f);
    float4 ip2jp2 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, 2).uv, 0.0f);

    float4 jm1 =
        interpolate_cubic(im1jm1, ijm1, ip1jm1, ip2jm1,
                          ratio.x);
    float4 j = interpolate_cubic(
        im1j, ij, ip1j, ip2j, ratio.x);
    float4 jp1 =
        interpolate_cubic(im1jp1, ijp1, ip1jp1, ip2jp1,
                          ratio.x);
    float4 jp2 =
        interpolate_cubic(im1jp2, ijp2, ip1jp2, ip2jp2,
                          ratio.x);

    return interpolate_cubic(jm1, j, jp1, jp2,
                             ratio.y);
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
  outputData[uint2(DTid.x, DTid.y)] = sample_cubic(inputData, samplerLinear, uv);// inputData.SampleLevel(samplerLinear, startingPoint, 0);
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

