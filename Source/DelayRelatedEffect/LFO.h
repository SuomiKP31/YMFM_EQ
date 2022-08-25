/*
  ==============================================================================

    LFO.h
    Created: 1 Apr 2022 11:31:28am
    Author:  KP31

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum LFOWaveform
{
	SineWave,
	Triangle
};
enum InterpolationMethod
{
	interpolationNearestNeighbour,
	interpolationLinear,
	interpolationCubic

};

class LFO
{
	// Low Frequency Oscillator, given phase, output a value inside a simulated waveform

public:
	static float lfo(float phase, LFOWaveform waveform);
};
