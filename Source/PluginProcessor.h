/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSPProcessorUnits/RepeatProcessor.h"
#include "DSPProcessorUnits/GateProcessor.h"
#include "DSPProcessorUnits/FlangerProcessor.h"
#include "DSPProcessorUnits/PhaserProcessor.h"
#include "DSPProcessorUnits/BitCrusherProcessor.h"
#include "DSPProcessorUnits/TapeStopProcessor.h"

enum Slope_Choice
{
	slope_12,
    slope_24,
    slope_36,
    slope_48,
};

enum ChainPosition
{
    LowCut,
    Peak,
    HighCut
};

struct ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    int lowCutSlope{ 0 }, highCutSlope{ 0 };
};

// ==== Aliases ====
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;  // Low, Peak, High
using Coefficient = Filter::CoefficientsPtr;

// ==== Free Functions ====
void UpdateCoefficients(Coefficient& old, const Coefficient& replacement);

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
Coefficient makePeakFilter(const ChainSettings& chain_settings, double sampleRate);

inline auto makeLowCutCoefficient(const ChainSettings& chain_settings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chain_settings.lowCutFreq,
        sampleRate,
        (1 + chain_settings.lowCutSlope) * 2);
}
inline auto makeHighCutCoefficient(const ChainSettings& chain_settings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chain_settings.highCutFreq,
        sampleRate,
        (1 + chain_settings.highCutSlope) * 2);
}
//-------------------------

template <int Index, typename ChainType, typename CoefficientType>
void EnableFilterInCutChain(ChainType& cutfilter, const CoefficientType& coefficient)
{
    UpdateCoefficients(cutfilter.template get<Index>().coefficients, coefficient[Index]);
    cutfilter.template setBypassed<Index>(false);
}

template <typename ChainType, typename CoefficientType>
void UpdateCutFilter(ChainType& cutfilter, const CoefficientType& coefficient,
    const int& slope)
{
    cutfilter.template setBypassed<0>(true);
    cutfilter.template setBypassed<1>(true);
    cutfilter.template setBypassed<2>(true);
    cutfilter.template setBypassed<3>(true);

    switch (slope)
    {
    case slope_48:
        EnableFilterInCutChain<3>(cutfilter, coefficient);
    case slope_36:
        EnableFilterInCutChain<2>(cutfilter, coefficient);
    case slope_24:
        EnableFilterInCutChain<1>(cutfilter, coefficient);
    case slope_12:
        EnableFilterInCutChain<0>(cutfilter, coefficient);
        break;
    default:
        break;

    }
}
//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "AudioParameter", createParameterLayout()};

	RepeatProcessor repeatProcessor;
	GateProcessor gateProcessor;
	FlangerProcessor flangerProcessor;
	PhaserProcessor phaserProcessor;
	BitCrusherProcessor bitcrusherProcessor;
	TapeStopProcessor tapestopProcessor;

private:


    MonoChain leftChain, rightChain;
    
    void UpdatePeakFilter(const ChainSettings& chain_settings);



    void UpdateFilters();
    void UpdateLowCutFilters(ChainSettings& chain_settings);
    void UpdateHighCutFilters(ChainSettings& chain_settings);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};



