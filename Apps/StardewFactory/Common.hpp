#ifndef sf_Common_hpp
#define sf_Common_hpp
#pragma once

using namespace m;

extern math::RandomGenerator g_numberGenerator;

#define FIELD_SIZE 15

#define INVENTORY_SIZE 5

static const Float s_matureAge                   = 3.0f;
static const Float s_seedPrice                   = 150.0f;
static const Float s_plantDeathRateWhenHarvested = 5.0f;
static const Float s_plantDeathRateWhenGrounded  = 10.0f;
static const Float s_plantBaseConsumptionRate    = 1.0f;
static const Float s_plantBaseHealth             = 100.0f;

static const Float s_fieldRegenerationRate = 0.3f;
static const Float s_fieldMaxNutiments     = 10.0f;

const static Float parcelSize    = 40;
const static Float parcelPadding = 3;

const static Float agentSizeSmall = 2;
const static Float agentSizeBig   = 10;

const static Float heroSize = 4;

static Float s_machineRefreshTime = 1.0f;

#endif
