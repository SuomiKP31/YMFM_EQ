/*
  ==============================================================================

    BitCrusherProcessor.h
    Created: 2 Apr 2022 5:48:27pm
    Author:  KP31

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class BitCrusherProcessor
{
public:
	int bitDepth = 8; // Reduce the bit depth to ...
	int rateDivide = 10; // Reduce sample rate by ... times

	void process(juce::AudioBuffer<float>& buffer, double sampleRate);
	void setBypass(bool bypassFlag);

private:
	bool bypass = true;
};
