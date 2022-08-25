/*
  ==============================================================================

    TimeDomainProcessingComponent.h
    Created: 14 Mar 2022 11:33:25am
    Author:  Roarlisf/KP31

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

//==============================================================================
/*
*/


class LabeledTextInputField : public juce::TextEditor
{
public:
    juce::String label;
    juce::Label labelComp;
    LengthAndCharacterRestriction lacr{ 5, "0123456789" };

    double getNumber();
    
};

class TriggerButton : public juce::TextButton
{
public:
	enum TriggerAction
	{
		Activate,
		Deactivate,
		None
	};

	juce::Atomic<bool> activating;

	virtual void OnStateChanged();
	virtual void TakeAction(TriggerAction &action) = 0;
	TriggerAction JudgeButtonAction(ButtonState currentState);

protected:
	ButtonState cachedState = buttonNormal;
};

class RepeatButton : public TriggerButton
{
public:
    int denominator;
    LabeledTextInputField* bpmInput;
    RepeatProcessor* repeatProcessor;

	void TakeAction(TriggerAction &action) override;

};

class GateButton : public TriggerButton
{
public:
	int denominator;
	LabeledTextInputField* bpmInput;
	GateProcessor* gateProcessor;

	void TakeAction(TriggerAction& action) override;

};

class FlangerButton : public TriggerButton
{
public:
	void TakeAction(TriggerAction& action) override;

	FlangerProcessor* flangerProcessor;
};

class PhaserButton : public TriggerButton
{
public:
	void TakeAction(TriggerAction& action) override;

	PhaserProcessor* phaserProcessor;
};

class BitCrusherButton : public TriggerButton
{
public:
	void TakeAction(TriggerAction& action) override;

	BitCrusherProcessor* bitcrusherProcessor;
};

class TapestopButton : public TriggerButton
{
public:
	void TakeAction(TriggerAction& action) override;

	TapeStopProcessor* tapestopProcessor;
};

class TimeDomainProcessingComponent  : public juce::Component
{
public:
    TimeDomainProcessingComponent(SimpleEQAudioProcessor&);
    ~TimeDomainProcessingComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    SimpleEQAudioProcessor& audioProcessor;

    LabeledTextInputField bpmInput;
    RepeatButton re8, re12, re16, re32;
	GateButton ga8, ga16, ga24, ga32;
	FlangerButton fl1;
	PhaserButton ph1;
	BitCrusherButton bc1;
	TapestopButton tp1;


private:
    std::vector<juce::Component*> getComponent();
	void setShortcuts();
	void initButtons();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeDomainProcessingComponent)
};


