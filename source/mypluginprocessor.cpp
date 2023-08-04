//------------------------------------------------------------------------
// Copyright(c) 2023 cma5.
//------------------------------------------------------------------------

#include "mypluginprocessor.h"
#include "myplugincids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "brownnoise.h"
#include <algorithm>

using namespace Steinberg;

namespace MyCompanyName {
//------------------------------------------------------------------------
// modeldProcessor
//------------------------------------------------------------------------
modeldProcessor::modeldProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kmodeldControllerUID);
}

//------------------------------------------------------------------------
modeldProcessor::~modeldProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* Deleted audio in since we don't need that right now */
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
					case GainParams::kParamGainId:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							(float) mGain = value;
						break;
					case GainParams::kParamAttackId:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							(float)fAttack = value;
						break;
					case GainParams::kParamSustainId:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							(float)fSustain = value;
						break;
					case GainParams::kParamDecayId:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							(float)fDecay = value;
						break;
					case FilterParams::kParamFilterId:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
							mFreq = value;
						break;
					case OscParams::kParamOscId:
						if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						fWaveSelect = value;
						break;
				}
			}
		}
	}
	//--- Second: Read input events-------------
	Vst::IEventList* eventList = data.inputEvents;
	if(eventList)
	{
		int32 numEvent = eventList->getEventCount();
		for (int32 i = 0; i < numEvent; i++)
		{
			Vst::Event event;
			if (eventList->getEvent(i, event) == kResultOk)
			{
				// here we do not take care of the channel info of the event
				switch (event.type)
				{
					//----------------------
				case Vst::Event::kNoteOnEvent:
					fPressed.push_back(event.noteOn.pitch); //Track if more than one key is pressed
					// use the velocity as gain modifier: a velocity max (1) will lead to silent audio
					fFrequency = 440.0f * powf(2.0, (float)(event.noteOn.pitch - 69) / 12.f);
					fDeltaAngle = PI2 * fFrequency / (float)data.processContext->sampleRate;
					mGainReduction = event.noteOn.velocity; // value between 0 and 1
					fVolume = 0.3f;
					fOscPhase = 0.f;
					break;

					//----------------------
				case Vst::Event::kNoteOffEvent:
					if (fPressed.back() == event.noteOff.pitch)
					{
						fPressed.pop_back();
						if (!fPressed.empty()) {
							fFrequency = 440.0f * powf(2.0, (float)(fPressed.back() - 69) / 12.f);
							fDeltaAngle = PI2 * fFrequency / (float)data.processContext->sampleRate;
						}
					}
					else {
						fPressed.erase(std::remove(fPressed.begin(), fPressed.end(), event.noteOff.pitch), fPressed.end());
					}
					if (fPressed.empty())//If no key is pressed
					{
						fVolume = 0.f;// noteOff reset the gain modifier
						mGainReduction = 0.f;
					}
					
					break;
				}
			}
		}
	}

	if (data.numSamples == 0)
		return kResultOk;
	
	//--- Here you have to implement your processing
	//int32 numChannels = data.inputs[0].numChannels;
	
	//---get audio buffers using helper-functions(vstaudioprocessoralgo.h)-------------
	/*uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
	void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);*/

	// Here could check the silent flags
	// now we will produce the output
	// mark our outputs has not silent
	data.outputs[0].silenceFlags = 0;


	Vst::Sample32* outL = data.outputs[0].channelBuffers32[0];
	Vst::Sample32* outR = data.outputs[0].channelBuffers32[1];
	for (int32 i = 0; i < data.numSamples; i++) {
		outL[i] = mGain * sin(fOscPhase);
		outL[i] *= fVolume;
		outR[i] = outL[i];

		fOscPhase += fDeltaAngle;
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API modeldProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace MyCompanyName
