//------------------------------------------------------------------------
// Copyright(c) 2023 cma5.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyCompanyName {
//------------------------------------------------------------------------
static const Steinberg::FUID kmodeldProcessorUID (0x693E2A0A, 0xA5495623, 0xB6D69DD2, 0x3B8FE3DF);
static const Steinberg::FUID kmodeldControllerUID (0x77CED473, 0xB1795E5E, 0xB3514452, 0xF809D310);

#define modeldVST3Category "Instrument"
enum GainParams : Steinberg::Vst::ParamID
{
	kParamGainId = 102, //should be an unique ID
	kParamAttackId = 104,
	kParamSustainId = 105,
	kParamDecayId = 106,
};

enum FilterParams : Steinberg::Vst::ParamID
{
	kParamFilterId = 103, //should be an unique ID
};

enum OscParams : Steinberg::Vst::ParamID
{
	kParamOscId = 107, //should be an unique ID
};

//------------------------------------------------------------------------
} // namespace MyCompanyName
