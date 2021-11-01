#ifndef sf_Common_hpp
#define sf_Common_hpp
#pragma once

#include <Math.hpp>

using namespace m;

extern math::mXoRandomNumberGenerator g_numberGenerator;

#define FIELD_SIZE 15

#define INVENTORY_SIZE 5

static const mFloat s_matureAge                   = 3.0f;
static const mFloat s_seedPrice                   = 150.0f;
static const mFloat s_plantDeathRateWhenHarvested = 5.0f;
static const mFloat s_plantDeathRateWhenGrounded  = 10.0f;
static const mFloat s_plantBaseConsumptionRate    = 1.0f;
static const mFloat s_plantBaseHealth             = 100.0f;

static const mFloat s_fieldRegenerationRate = 0.3f;
static const mFloat s_fieldMaxNutiments     = 10.0f;

const static mFloat parcelSize    = 40;
const static mFloat parcelPadding = 3;

const static mFloat agentSizeSmall = 2;
const static mFloat agentSizeBig   = 10;

const static mFloat heroSize = 4;

static mFloat s_machineRefreshTime = 1.0f;

#endif
