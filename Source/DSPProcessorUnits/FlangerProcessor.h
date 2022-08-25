/*
  ==============================================================================

    FlangerProcessor.h
    Created: 1 Apr 2022 11:31:52am
    Author:  KP31

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../DelayRelatedEffect/LFO.h"


class FlangerProcessor
{
public:
	FlangerProcessor();
	~FlangerProcessor();

	void process(juce::AudioBuffer<float>& buffer, double sampleRate);
	void setBypass(bool bypassFlag);


	void setupDelayBuffer(int channel, double sampleRate);

	float lfoFrequency = 0.5f;
	LFOWaveform lfoWaveform = SineWave;
	InterpolationMethod interpolation = interpolationLinear;
private:
	bool bypass = true;

	juce::AudioSampleBuffer delayBuffer;
	int delayWritePosition = 0;
	int delayBufferSamples = 0;

	float delayTime = 0.005f; //5ms
	float width = 0.004f; //4ms /width has to be smaller than delay time, otherwise you'll get empty samples 

	float depth = 1.0f;
	
	float feedback = 0.25f;

	float sampleRate = 48000.0;
	float inverseSampleRate = 1 / 48000.0;
	float lfoPhase = 0.0f;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerProcessor)
};