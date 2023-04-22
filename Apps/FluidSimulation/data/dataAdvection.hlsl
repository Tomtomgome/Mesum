#include "commonInclude.hlsl"

Texture2D<float4> inputData : register(t0);
Texture2D<float2> inputVelocity : register(t1);
RWTexture2D<float4> outputData : register(u0);

float2 sample_velocity(CoordData a_uv)
{
    float2 velocity;
    velocity.x = inputVelocity.SampleLevel(samplerLinearBlackBorder, uv_plusHalf(a_uv, -1, 0).uv, 0).x;
    velocity.y = inputVelocity.SampleLevel(samplerLinearBlackBorder, uv_plusHalf(a_uv, 0, -1).uv, 0).y;
    return velocity;
}

// ---------- Advection
[numthreads( COMPUTE_GROUP_SIZE, COMPUTE_GROUP_SIZE, 1 )]
void cs_advect(uint3 DTid : SV_DispatchThreadID)
{
    // IMPROVE compute index
    CoordData uv = compute_uv(DTid);

    if(DTid.x == 0 || DTid.y == 0 || DTid.x >= data.resolution.x -1 || DTid.y >= data.resolution.y - 1)
    //if(DTid.x >= data.resolution.x || DTid.y >= data.resolution.y)
    {
        return;
    }

    // Need to separate center cell quantities advection from staggered quantities advections
    float2 velocity = sample_velocity(uv);

    float2 startingPoint = uv.uv - (g_time * velocity / data.cellSize) * uv.pixel;
    uv.uv = startingPoint;

    outputData[uint2(DTid.x, DTid.y)] = sample_cubic_f4(inputData, samplerLinearWrap, uv);

    // ---------
    float ratioVapor        = outputData[uint2(DTid.x, DTid.y)].x;
    float ratioCloud    = outputData[uint2(DTid.x, DTid.y)].y;
    float ratioRain         = outputData[uint2(DTid.x, DTid.y)].z;
    float potentialTemp     = outputData[uint2(DTid.x, DTid.y)].a;

    // Altitude
    float altitude = 0.5 * data.cellSize.y + DTid.y * data.cellSize.y; // m

    // Pressure
    float pressure = pow((1 - g_lapseRate*altitude/g_temperatureGround), 5.2561);


    float moleFractionVapor = ratioVapor/(ratioVapor + 1);
    float molarMassThermal = moleFractionVapor*g_molarMassWater + (1-moleFractionVapor)*g_molarMassAir; // g/mol
    float massFractionVapor = moleFractionVapor*g_molarMassWater/molarMassThermal;

    float isentropicThermal = massFractionVapor*g_isentropicWater + (1-massFractionVapor)*g_isentropicAir;
    float temperatureThermal = g_temperatureGround * pow((pressure/g_pressureGround), (isentropicThermal-1.0f/isentropicThermal));

    float potentialTempCelcius = temperatureThermal - 273.15;
    float saturationMixingRatio = (0.03801664/pressure)*exp(17.67*potentialTempCelcius/(potentialTempCelcius+243.50));

    float Er = 0; // ??
    float Ac = 0; // ??
    float Kc = 0; // ??
    float latent = 2.5; // ??
    outputData[uint2(DTid.x, DTid.y)].x = ratioVapor + min(saturationMixingRatio - ratioVapor, ratioCloud) + Er;
    outputData[uint2(DTid.x, DTid.y)].y = ratioCloud - min(saturationMixingRatio - ratioVapor, ratioCloud) - Ac - Kc;
    outputData[uint2(DTid.x, DTid.y)].z = ratioRain + Ac + Kc - Er;
    ratioVapor = outputData[uint2(DTid.x, DTid.y)].x;
    ratioCloud = outputData[uint2(DTid.x, DTid.y)].y;
    ratioRain = outputData[uint2(DTid.x, DTid.y)].z;


    moleFractionVapor = ratioVapor/(ratioVapor + 1);
    molarMassThermal = moleFractionVapor*g_molarMassWater + (1-moleFractionVapor)*g_molarMassAir; // g/mol
    massFractionVapor = moleFractionVapor*g_molarMassWater/molarMassThermal;

    isentropicThermal = massFractionVapor*g_isentropicWater + (1-massFractionVapor)*g_isentropicAir;

    float heatCapacity = isentropicThermal*g_gasConstant/(molarMassThermal*(isentropicThermal-1));
    float cloud = -min(saturationMixingRatio - ratioVapor, ratioCloud);
    float moleFractionCloud = ratioCloud/(ratioCloud + 1);
    potentialTemp = outputData[uint2(DTid.x, DTid.y)].a = potentialTemp + (latent/heatCapacity)*moleFractionCloud;
    // qvs ← getSaturationRatio(getAbsoluteTemperature(θ ), p∞) //see Eq. (16) and Eq. (28)
    //qv ← qv + min(qvs − qv, qc ) + Er
    //qc ← qc − min(qvs − qv, qc ) − Ac − Kc
    //qr ← qr + Ac + Kc − Er
    //Xv ← getMoleFraction(qv ) //see Eq. (9)
    //Mth ← getAverageMolarMass(Xv ) //see Eq. (7)
    //γth ← getIsentropicExponent(Xv MW/Mth) //see Eq. (11)
    //cth_p ← getHeatCapacity(γth, Mth) //see Eq. (18)
    //θ ← θ + L/cth_p · getMoleFraction(−min(qvs − qv, qc )) //see Eq. (9)
}
