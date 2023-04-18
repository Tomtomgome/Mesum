#include "commonInclude.hlsl"

Texture2D<float4> inputData : register(t0);
Texture2D<float2> inputVelocity : register(t1);
RWTexture2D<float2> outputVelocity : register(u0);
RWTexture2D<float4> outputDebug : register(u1);

float2 sample_velocity(CoordData a_uv)
{
    float2 velocity;
    velocity.x = inputVelocity.SampleLevel(samplerLinear, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
    velocity.y = inputVelocity.SampleLevel(samplerLinear, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
    return velocity;
}

// ---------- applyForces
static const float g_alpha             = -2;
static const float g_beta              = 6.5;
static const float g_vorticityStrength = 0.3;

static const float g_ambientT = 270;

float compute_vorticity(CoordData a_uv)
{
    float2 speed_dx =
        sample_velocity(uv_plus_res(a_uv, 1, 0)) -
        sample_velocity(uv_plus_res(a_uv, -1, 0));
    float dv_dx = speed_dx.y / (2 * data.cellSize.y);
    float2 speed_dy =
        sample_velocity(uv_plus_res(a_uv, 0, 1)) -
        sample_velocity(uv_plus_res(a_uv, 0, -1));
    float du_dy = speed_dy.x / (2 * data.cellSize.x);

    return dv_dx - du_dy;
}

float2 compute_vorticityForce(CoordData a_uv)
{
    float input_iplusone = abs(compute_vorticity(uv_plus_res(a_uv, 1, 0)));
    float input_iminusone = abs(compute_vorticity(uv_plus_res(a_uv, -1, 0)));
    float input_jplusone = abs(compute_vorticity(uv_plus_res(a_uv, 0, 1)));
    float input_jminusone = abs(compute_vorticity(uv_plus_res(a_uv, 0, -1)));

    float input_ij = compute_vorticity(a_uv);

    float2 outGradient;
    outGradient.x = (input_iplusone - input_iminusone) / (2 * data.cellSize.x);
    outGradient.y = (input_jplusone - input_jminusone) / (2 * data.cellSize.y);

    float gradientSqLen = dot(outGradient, outGradient);
    float len = max(sqrt(gradientSqLen), 0.0000001);
    outGradient = outGradient / len;
    outGradient = float2(outGradient.y * input_ij, outGradient.x * input_ij);

    return outGradient;
}

[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_simulation(uint3 DTid : SV_DispatchThreadID)
{
    // Constants
    static const float gravity           = -9.81; // m/s^2

    static const float transitionHeight = 8000; // m
    static const float temperatureGround = 300; // K
    static const float lapseRate = -0.0065f   ; // K/m

    static const float isentropicAir = 1.4f;//
    static const float isentropicWater = 1.33;

    static const float pressureGround = 1.0f; // bar

    static const float molarMassAir = 28.96f; // g/mol
    static const float molarMassWater = 18.02f; // g/mol

    static const float densityDryAir = 1.2754f; // kg/m^3

    // IMPROVE compute index
    CoordData uv = compute_uv(DTid);
    if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
      return;
    }

    float4 modelData = inputData.SampleLevel(samplerLinear, uv_plusHalf(uv, 0, 1).uv, 0);
    float massVapor        = modelData.x;
    float massCondensed    = modelData.y;
    float massRain         = modelData.z;

    // Water fraction
    //float massDryAir = densityDryAir * data.cellSize.x * data.cellSize.y * 1.0f;
    //float massRatioVapor = massVapor/massDryAir;
    //float moleFractionWater = massRatioVapor/(massRatioVapor+1.0f); // Either this or a ratio

    float moleFractionWater = massVapor/(massVapor+1.0f); // Either this or a ratio
    float molarMassThermal = moleFractionWater*molarMassWater + (1-moleFractionWater)*molarMassAir; // g/mol
    float massFractionWater = moleFractionWater*molarMassWater/molarMassThermal;

    // Altitude
    float altitude = 0.5f * data.cellSize.y + DTid.y * data.cellSize.y; // m

    // Pressure
    float pressure = pressureGround * pow((1 + lapseRate*altitude/temperatureGround), 5.2561);

    // Temperature
    float temperatureAir;
    if(altitude <= transitionHeight)
    {
        temperatureAir = temperatureGround + altitude * lapseRate;
    }
    else
    {
        temperatureAir = temperatureGround + transitionHeight * lapseRate - (altitude - transitionHeight) * lapseRate;
    }

    float isentropicThermal = massFractionWater*isentropicWater + (1-massFractionWater)*isentropicAir;
    float temperatureThermal = temperatureGround * pow((pressure/pressureGround), (isentropicThermal-1.0f/isentropicThermal));

    // base copy
    outputVelocity[uint2(DTid.x, DTid.y)] = inputVelocity.SampleLevel(samplerPoint, uv.uv, 0);
  
    // gravity
    //outputVelocity[uint2(DTid.x, DTid.y)].y += g_time * g_gravity;

    // Boyancy
    float boyancy = gravity*((molarMassAir/molarMassThermal)*(temperatureThermal/temperatureAir)-1);
    //outputDebug[uint2(DTid.x, DTid.y)].x = boyancy;
    //outputDebug[uint2(DTid.x, DTid.y)].y = (molarMassAir/molarMassThermal);
    //outputDebug[uint2(DTid.x, DTid.y)].z = ((molarMassAir/molarMassThermal)*100) % 1.0;

    outputVelocity[uint2(DTid.x, DTid.y)].y += g_time * boyancy;

    // Vorticity Confinment
    float2 vorticityForceX =
                    0.5 * 
                    (compute_vorticityForce(uv) + 
                    compute_vorticityForce(uv_plus_res(uv, 1, 0)));
    float2 vorticityForceY =
                    0.5 *
                    (compute_vorticityForce(uv) + 
                    compute_vorticityForce(uv_plus_res(uv, 0, 1)));

    float2 vorticityMultiplier = g_time * g_vorticityStrength * data.cellSize;
    //outputVelocity[uint2(DTid.x, DTid.y)].x += vorticityMultiplier.x * vorticityForceX.x;
    //outputVelocity[uint2(DTid.x, DTid.y)].y += vorticityMultiplier.y * vorticityForceY.y;
}

