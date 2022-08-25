/*
  ==============================================================================

    PhaserProcessor.h
    Created: 2 Apr 2022 11:12:44am
    Author:  Suomi
    
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../DelayRelatedEffect/LFO.h"



class PhaserProcessor
{
	
public:
	
	PhaserProcessor();
	~PhaserProcessor();

	void process(juce::AudioBuffer<float>& buffer, double sampleRate);
	void setBypass(bool bypassFlag);


	void setupFilters(int channel, double sampleRate);

	class Filter : public juce::IIRFilter
	{
	public:
		void updateCoefficients(const double discreteFrequency) noexcept
		{
			jassert(discreteFrequency > 0);

			double wc = juce::jmin(discreteFrequency, juce::MathConstants<double>::pi * 0.99);
			double tan_half_wc = tan(wc / 2.0);

			coefficients = juce::IIRCoefficients(/* b0 */ tan_half_wc - 1.0,
				/* b1 */ tan_half_wc + 1.0,
				/* b2 */ 0.0,
				/* a0 */ tan_half_wc + 1.0,
				/* a1 */ tan_half_wc - 1.0,
				/* a2 */ 0.0);

			setCoefficients(coefficients);
		}
	};

	float lfoFrequency = 0.5f;
	float minFrequency = 80.0f; //Hz, should be above 20Hz 
	LFOWaveform lfoWaveform = SineWave;
	InterpolationMethod interpolation = interpolationLinear;



	juce::OwnedArray<Filter> filters;
	juce::Array<float> filteredOutputs;
	void updateFilters(double centerFrequency);

	int numFiltersPerChannel = 4;
	unsigned int sampleCountToUpdateFilters;
	unsigned int updateFiltersInterval;

private:
	bool bypass = true;


	float sweepWidth = 1000.0f; //Hz, in float
	float depth = 1.0f;

	float feedback = 0.05f;

	float sampleRate = 48000.0;
	float inverseSampleRate = 1 / 48000.0;
	float lfoPhase = 0.0f;
	double twoPi = 6.28;



	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserProcessor)
};