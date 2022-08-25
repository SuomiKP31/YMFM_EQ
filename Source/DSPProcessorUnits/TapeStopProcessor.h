/*
  ==============================================================================

    TapeStopProcessor.h
    Created: 4 Apr 2022 9:59:53am
    Author:  KP31

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../TapeStop/TapeStop.h"

class TapeStopProcessor
{
public:
	void process(juce::AudioBuffer<float>& buffer, double sampleRate);
	void setupTapeBuffer(int channel, double sampleRate);
	void setBypass(bool bypassFlag);

	juce::AudioSampleBuffer tapeBuffer;
	bool bypass = true;

	float sampleIdx = 0.f; // use this float to simulate a momentum attenuation of the tape
	double sampleRate = 48000;

	int sampleLingerLength = 6000;  //  How many samples we want this effect to last before muting the output
	float stopTime = 0.3f;

	std::vector<TapeStop> tapeMono;
};