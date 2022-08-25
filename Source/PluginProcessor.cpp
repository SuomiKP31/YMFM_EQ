/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return "YMFM";
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;

    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);
	flangerProcessor.setupDelayBuffer(getTotalNumInputChannels(), getSampleRate());
	phaserProcessor.setupFilters(getTotalNumInputChannels(), getSampleRate());
	tapestopProcessor.setupTapeBuffer(getTotalNumInputChannels(), getSampleRate());
    UpdateFilters();
}


void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    repeatProcessor.resizeRepeatProcessorByChannelNum(totalNumInputChannels);
	gateProcessor.resizeGateProcessorByChannelNum(totalNumInputChannels);
	
    UpdateFilters();

    juce::dsp::AudioBlock<float> block(buffer);

    auto left_block = block.getSingleChannelBlock(0);
    auto right_block = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> left_context(left_block);
    juce::dsp::ProcessContextReplacing<float> right_context(right_block);


	auto sampleRate = getSampleRate();
    repeatProcessor.process(buffer, sampleRate);
	gateProcessor.process(buffer, sampleRate);
	flangerProcessor.process(buffer, sampleRate);
	phaserProcessor.process(buffer, sampleRate);
	bitcrusherProcessor.process(buffer, sampleRate);
	tapestopProcessor.process(buffer, sampleRate);

	// Filters should run last
	leftChain.process(left_context);
	rightChain.process(right_context);
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
	return new SimpleEQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        UpdateFilters();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings setting;

    setting.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    setting.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    setting.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    setting.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    setting.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    setting.lowCutSlope = apvts.getRawParameterValue("LowCut Slope")->load();
    setting.highCutSlope = apvts.getRawParameterValue("HighCut Slope")->load();

    return setting;
}

Coefficient makePeakFilter(const ChainSettings& chain_settings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chain_settings.peakFreq,
        chain_settings.peakQuality, juce::Decibels::decibelsToGain(chain_settings.peakGainInDecibels));
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "HPF",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20.f));  // Low Cut Filter = High Pass Filter HPF
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "LPF",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20000.f));  // High Cut Filter = Low Pass Filter LPF
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq", "Peak Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, .5f, 1),
        0.0f));    // Gain is represented in decibels
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality", "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.f, .05f, 1),
        1.0f));    // Bandwidth of Peak

    juce::StringArray string_array;
    for( int i = 0; i < 4; i ++)
    {
        juce::String str;
        str << 12 + i * 12;
        str << " db/Oct";
        string_array.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut(HP) Slope", string_array, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut(LP) Slope", string_array, 0));

    return layout;
}

void SimpleEQAudioProcessor::UpdatePeakFilter(const ChainSettings& chain_settings)
{
    auto peak_coef = makePeakFilter(chain_settings, getSampleRate());

    *leftChain.get<Peak>().coefficients = *peak_coef;
    *rightChain.get<Peak>().coefficients = *peak_coef;
}

void UpdateCoefficients(Coefficient& old, const Coefficient& replacement)
{
    *old = *replacement;
}

void SimpleEQAudioProcessor::UpdateFilters()
{
    auto chain_settings = getChainSettings(apvts);
    UpdatePeakFilter(chain_settings);

    UpdateLowCutFilters(chain_settings);

    UpdateHighCutFilters(chain_settings);
}

void SimpleEQAudioProcessor::UpdateLowCutFilters(ChainSettings& chain_settings)
{
    auto cut_coef = makeLowCutCoefficient(chain_settings, getSampleRate());

    auto& lefthpf = leftChain.get<LowCut>();
    auto& righthpf = rightChain.get<LowCut>();
    UpdateCutFilter(lefthpf, cut_coef, chain_settings.lowCutSlope);
    UpdateCutFilter(righthpf, cut_coef, chain_settings.lowCutSlope);
}

void SimpleEQAudioProcessor::UpdateHighCutFilters(ChainSettings& chain_settings)
{
    auto cut_hcoef = makeHighCutCoefficient(chain_settings, getSampleRate());

    auto& leftlpf = leftChain.get<HighCut>();
    auto& rightlpf = rightChain.get<HighCut>();
    UpdateCutFilter(leftlpf, cut_hcoef, chain_settings.highCutSlope);
    UpdateCutFilter(rightlpf, cut_hcoef, chain_settings.highCutSlope);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
