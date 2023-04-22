SamplerState samplerLinear : register(s0);
SamplerState samplerPoint : register(s1);

SamplerState samplerPointBlackBorder : register(s2);
SamplerState samplerLinearBlackBorder : register(s3);

SamplerState samplerLinearWrap : register(s4);

struct Data
{
  uint2 resolution;
  float2 cellSize;
  uint wallAtTop;
  uint wallAtRight;
  uint wallAtBottom;
  uint wallAtLeft;
};
ConstantBuffer<Data> data : register(b0);

#define COMPUTE_GROUP_SIZE 16

static const float g_time = 1.0f; // s

static const float g_density = 1.0f;

static const float g_molarMassAir = 28.96f; // g/mol
static const float g_molarMassWater = 18.02f; // g/mol

static const float g_temperatureGround = 265; // K
static const float g_lapseRate = 0.0065f   ; // K/m

static const float g_isentropicAir = 1.4f;//
static const float g_isentropicWater = 1.33;

static const float g_gasConstant = 8314.0f; // J/g.mol

static const float g_pressureGround = 1.0f; // bar

struct CoordData
{
  float2 uv;
  float2 pixel;
  float2 halfPixel;
};

CoordData compute_uv(uint3 a_DTid)
{
  CoordData uv;
  
  uint dimX = data.resolution.x;
  uint dimY = data.resolution.y;

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

CoordData uv_plus_res(CoordData a_uv, int a_nbX, int a_nbY)
{
  CoordData result = a_uv;
  result.uv += float2(a_nbX * a_uv.pixel.x, a_nbY * a_uv.pixel.y);

  // This isn't very optimal :( maybe use a define instead, not dymanic but more performant
  if(data.wallAtRight)
  {
    result.uv.x = min(1.0f - a_uv.halfPixel.x, result.uv.x);
  }
  if(data.wallAtLeft)
  {
    result.uv.x = max(a_uv.halfPixel.x, result.uv.x);
  }
  if(data.wallAtTop)
  {
    result.uv.y = min(1.0f - a_uv.halfPixel.y, result.uv.y);
  }
  if(data.wallAtRight)
  {
    result.uv.y = max(a_uv.halfPixel.y, result.uv.y);
  }
  return result;
}

// Cubic Interpolation
void restrict_f4(inout float4 a_p, in float4 a_delta)
{
    a_p.x = (a_p.x * a_delta.x) > 0.0 ? a_p.x : 0.0;
    a_p.y = (a_p.y * a_delta.y) > 0.0 ? a_p.y : 0.0;
    a_p.z  = (a_p.z * a_delta.z) > 0.0 ? a_p.z : 0.0;
    a_p.w  = (a_p.w * a_delta.w) > 0.0 ? a_p.w : 0.0;
}

float4 interpolate_cubic_f4(float4 a_im1, float4 a_i, float4 a_ip1, float4 a_ip2, float a_alpha)
{
    float4 di        = 0.5 * (a_ip1 - a_im1);
    float4 diPlusOne = 0.5 * (a_ip2 - a_i);

    float4 delta = (a_ip1 - a_i);

    restrict_f4(di, delta);
    restrict_f4(diPlusOne, delta);

    float4 output =
        a_i + a_alpha * di +
        a_alpha * a_alpha * (3.0 * delta - 2.0 * di - diPlusOne) +
        a_alpha * a_alpha * a_alpha * (-2.0 * delta + di + diPlusOne);

    return output;
}

float4 sample_cubic_f4(in Texture2D<float4> a_tex, in SamplerState a_linearSampler, in CoordData a_uv)// in float2 uv, in float2 texSize)
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
        interpolate_cubic_f4(im1jm1, ijm1, ip1jm1, ip2jm1,
                          ratio.x);
    float4 j = interpolate_cubic_f4(
        im1j, ij, ip1j, ip2j, ratio.x);
    float4 jp1 =
        interpolate_cubic_f4(im1jp1, ijp1, ip1jp1, ip2jp1,
                          ratio.x);
    float4 jp2 =
        interpolate_cubic_f4(im1jp2, ijp2, ip1jp2, ip2jp2,
                          ratio.x);

    return interpolate_cubic_f4(jm1, j, jp1, jp2,
                             ratio.y);
}

void restrict_f2(inout float2 a_p, in float2 a_delta)
{
    a_p.x = (a_p.x * a_delta.x) > 0.0 ? a_p.x : 0.0;
    a_p.y = (a_p.y * a_delta.y) > 0.0 ? a_p.y : 0.0;
}

float2 interpolate_cubic_f2(float2 a_im1, float2 a_i, float2 a_ip1, float2 a_ip2, float a_alpha)
{
    float2 di        = 0.5 * (a_ip1 - a_im1);
    float2 diPlusOne = 0.5 * (a_ip2 - a_i);

    float2 delta = (a_ip1 - a_i);

    restrict_f2(di, delta);
    restrict_f2(diPlusOne, delta);

    float2 output =
        a_i + a_alpha * di +
        a_alpha * a_alpha * (3.0 * delta - 2.0 * di - diPlusOne) +
        a_alpha * a_alpha * a_alpha * (-2.0 * delta + di + diPlusOne);

    return output;
}

float2 sample_cubic_f2(in Texture2D<float2> a_tex, in SamplerState a_linearSampler, in CoordData a_uv)// in float2 uv, in float2 texSize)
{
  // We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
  // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
  // location [1, 1] in the grid, where [0, 0] is the top left corner.
    float2 samplePos = a_uv.uv / a_uv.pixel;// texSize;
    float2 texPosij = floor(samplePos - 0.5f) + 0.5f;

    float2 ratio = saturate(samplePos - texPosij);

    CoordData uv_ij = a_uv;
    uv_ij.uv = texPosij * a_uv.pixel;

    float2 im1jm1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, -1).uv, 0.0f);
    float2 ijm1   = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, -1).uv, 0.0f);
    float2 ip1jm1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, -1).uv, 0.0f);
    float2 ip2jm1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, -1).uv, 0.0f);

    float2 im1j = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, 0).uv, 0.0f);
    float2 ij = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, 0).uv, 0.0f);
    float2 ip1j = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, 0).uv, 0.0f);
    float2 ip2j = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, 0).uv, 0.0f);

    float2 im1jp1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, 1).uv, 0.0f);
    float2 ijp1   = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, 1).uv, 0.0f);
    float2 ip1jp1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, 1).uv, 0.0f);
    float2 ip2jp1 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, 1).uv, 0.0f);

    float2 im1jp2 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, -1, 2).uv, 0.0f);
    float2 ijp2   = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 0, 2).uv, 0.0f);
    float2 ip1jp2 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 1, 2).uv, 0.0f);
    float2 ip2jp2 = a_tex.SampleLevel(a_linearSampler, uv_plus(uv_ij, 2, 2).uv, 0.0f);

    float2 jm1 =
        interpolate_cubic_f2(im1jm1, ijm1, ip1jm1, ip2jm1,
                          ratio.x);
    float2 j = interpolate_cubic_f2(
        im1j, ij, ip1j, ip2j, ratio.x);
    float2 jp1 =
        interpolate_cubic_f2(im1jp1, ijp1, ip1jp1, ip2jp1,
                          ratio.x);
    float2 jp2 =
        interpolate_cubic_f2(im1jp2, ijp2, ip1jp2, ip2jp2,
                          ratio.x);

    return interpolate_cubic_f2(jm1, j, jp1, jp2,
                             ratio.y);
}