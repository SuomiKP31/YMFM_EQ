/*
  ==============================================================================

    TimeDomainProcessingComponent.cpp
    Created: 14 Mar 2022 11:33:25am
    Author:  Roarlisf/KP31

  ==============================================================================
*/

#include <JuceHeader.h>
#include "TimeDomainProcessingComponent.h"


double LabeledTextInputField::getNumber()
{
    auto s = getText().toStdString();
    if(s.length()==0)
    {
        return 120;
    }
    return std::stof(s);
}

//-----------------TriggerButton

void TriggerButton::OnStateChanged()
{
	TriggerAction action;
	action = JudgeButtonAction(getState());
	TakeAction(action);

}


TriggerButton::TriggerAction TriggerButton::JudgeButtonAction(ButtonState currentState)
{
	// DBG(juce::String::formatted("cache %d state %d", cachedState, currentState));
	if (cachedState == buttonNormal)
	{
		cachedState = currentState;
		if (currentState == buttonOver)
		{
			return None;
		}
		if (currentState == buttonDown)
		{
			return Activate;
		}
		return None;
	}
	if (cachedState == buttonOver)
	{
		cachedState = currentState;
		if (currentState == buttonNormal)
		{
			return None;
		}
		if (currentState == buttonDown)
		{
			return Activate;
		}
		return None;
	}
	if (cachedState == buttonDown)
	{
		cachedState = currentState;
		if (currentState == buttonDown)
		{
			return None;
		}

		return Deactivate;
	}
	return None;
}

//---------------------RepeatButton : TriggerButton
void RepeatButton::TakeAction(TriggerAction &action)
{
	switch (action)
	{
	case Activate: {
		repeatProcessor->setBpm(bpmInput->getNumber());
		//DBG(bpmInput->getNumber());
		repeatProcessor->setInterval(denominator);
		break;
	}
	case Deactivate: {
		repeatProcessor->setInterval(0);
		break;
	}
	case None: break;
	default: break;
	}
}


void GateButton::TakeAction(TriggerAction& action)
{
	switch (action)
	{
	case Activate: {
		gateProcessor->setBpm(bpmInput->getNumber());
		//DBG(bpmInput->getNumber());
		gateProcessor->setInterval(denominator);
		break;
	}
	case Deactivate: {
		gateProcessor->setInterval(0);
		break;
	}
	case None: break;
	default: break;
	}
}

void FlangerButton::TakeAction(TriggerAction& action)
{
	switch (action)
	{
	case Activate: {
		flangerProcessor->setBypass(false);
		break;
	}
	case Deactivate: {
		flangerProcessor->setBypass(true);
		break;
	}
	case None: break;
	default: break;
	}
}

void PhaserButton::TakeAction(TriggerAction& action)
{
	switch (action)
	{
	case Activate: {
		phaserProcessor->setBypass(false);
		break;
	}
	case Deactivate: {
		phaserProcessor->setBypass(true);
		break;
	}
	case None: break;
	default: break;
	}
}

void BitCrusherButton::TakeAction(TriggerAction& action)
{
	switch (action)
	{
	case Activate: {
		bitcrusherProcessor->setBypass(false);
		break;
	}
	case Deactivate: {
		bitcrusherProcessor->setBypass(true);
		break;
	}
	case None: break;
	default: break;
	}
}

//==============================================================================

void TapestopButton::TakeAction(TriggerAction& action)
{
	switch (action)
	{
	case Activate: {
		tapestopProcessor->setBypass(false);
		break;
	}
	case Deactivate: {
		tapestopProcessor->setBypass(true);
		break;
	}
	case None: break;
	default: break;
	}
}

TimeDomainProcessingComponent::TimeDomainProcessingComponent(SimpleEQAudioProcessor&p) : audioProcessor(p)
{

    bpmInput.setInputFilter(&bpmInput.lacr, false);

  
    for(auto* comp : getComponent())
    {
        addAndMakeVisible(comp);
    }

	initButtons();

	setShortcuts();
}

TimeDomainProcessingComponent::~TimeDomainProcessingComponent()
{


}

void TimeDomainProcessingComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
}

void TimeDomainProcessingComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    auto bound = getLocalBounds();
	auto bpmBound = bound.removeFromLeft(bound.getWidth() * 0.33);
	bpmInput.label = "BPM";
    bpmInput.labelComp.setBounds(bpmBound.removeFromTop(bpmBound.getHeight() * 0.33));
    bpmInput.labelComp.setText(bpmInput.label, juce::dontSendNotification);
    bpmInput.setBounds(bpmBound);

	auto buttonW = bound.getWidth() * 0.25;
    auto repeatBound = bound.removeFromLeft(buttonW);
    auto repeatH = repeatBound.getHeight() * 0.25;

	auto gateBound = bound.removeFromLeft(buttonW);
	auto gateH = gateBound.getHeight() * 0.25;

	auto flanBound = bound.removeFromLeft(buttonW);
	auto flanH = flanBound.getHeight() * 0.25;

    std::vector tbtn{ &re8, &re12, &re16, &re32 };
    for (int i = 0; i < 4; i++)
    {
        tbtn[i]->setBounds(repeatBound.removeFromTop(repeatH));
        
    }
	std::vector gbtn = { &ga8, &ga16, &ga24, &ga32 };
	for (int i = 0; i < 4; i++)
	{
		gbtn[i]->setBounds(gateBound.removeFromTop(gateH));
	}

	fl1.setBounds(flanBound.removeFromTop(flanH));
	ph1.setBounds(flanBound.removeFromTop(flanH));
	bc1.setBounds(flanBound.removeFromTop(flanH));
	tp1.setBounds(flanBound.removeFromTop(flanH));
}


std::vector<juce::Component*> TimeDomainProcessingComponent::getComponent()
{
    return {
        &bpmInput,
        &bpmInput.labelComp,
        &re8,&re12,&re16,&re32,
		&ga8,&ga16,&ga24,&ga32,
		&fl1,&ph1,&bc1,&tp1
    };
}

void TimeDomainProcessingComponent::setShortcuts()
{
	fl1.addShortcut(juce::KeyPress('z', juce::ModifierKeys::shiftModifier, 0));
	ph1.addShortcut(juce::KeyPress('x', juce::ModifierKeys::shiftModifier, 0));
	bc1.addShortcut(juce::KeyPress('c', juce::ModifierKeys::shiftModifier, 0));
	tp1.addShortcut(juce::KeyPress('v', juce::ModifierKeys::shiftModifier, 0));

	re8.addShortcut(juce::KeyPress('q', juce::ModifierKeys::shiftModifier, 0));
	re12.addShortcut(juce::KeyPress('w', juce::ModifierKeys::shiftModifier, 0));
	re16.addShortcut(juce::KeyPress('e', juce::ModifierKeys::shiftModifier, 0));
	re32.addShortcut(juce::KeyPress('r', juce::ModifierKeys::shiftModifier, 0));

	ga8.addShortcut(juce::KeyPress('a', juce::ModifierKeys::shiftModifier, 0));
	ga16.addShortcut(juce::KeyPress('s', juce::ModifierKeys::shiftModifier, 0));
	ga24.addShortcut(juce::KeyPress('d', juce::ModifierKeys::shiftModifier, 0));
	ga32.addShortcut(juce::KeyPress('f', juce::ModifierKeys::shiftModifier, 0));
}

void TimeDomainProcessingComponent::initButtons()
{
	std::vector tbtn{ &re8, &re12, &re16, &re32 };
	std::vector suffix{ 8, 12, 16, 32 };
	for (int i = 0; i < 4; i++)
	{
		char buttonText[20];
		sprintf(buttonText, "RE%d", suffix[i]);
		auto& btn = tbtn[i];
		btn->setButtonText(buttonText);
		btn->onStateChange = [btn] {btn->OnStateChanged(); };
		btn->denominator = suffix[i];
		btn->repeatProcessor = &audioProcessor.repeatProcessor;
		btn->bpmInput = &bpmInput;
	}

	std::vector gbtn{ &ga8, &ga16, &ga24, &ga32 }; // GA24+ sounds really bad
	suffix.assign({ 4, 8, 12, 16 });


	for (int i = 0; i < 4; i++)
	{
		char buttonText[20];
		sprintf(buttonText, "GA%d", suffix[i]);
		auto& btn = gbtn[i];
		btn->setButtonText(buttonText);
		btn->onStateChange = [btn] {btn->OnStateChanged(); };
		btn->denominator = suffix[i];
		btn->gateProcessor = &audioProcessor.gateProcessor;
		btn->bpmInput = &bpmInput;
	}

	fl1.onStateChange = [btn = &fl1] {btn->OnStateChanged();  };
	fl1.setButtonText("Flanger");
	fl1.flangerProcessor = &audioProcessor.flangerProcessor;


	ph1.onStateChange = [btn = &ph1] {btn->OnStateChanged(); };
	ph1.setButtonText("Phaser");
	ph1.phaserProcessor = &audioProcessor.phaserProcessor;

	bc1.onStateChange = [btn = &bc1] {btn->OnStateChanged();  };
	bc1.setButtonText("BitCrusher");
	bc1.bitcrusherProcessor = &audioProcessor.bitcrusherProcessor;

	tp1.onStateChange = [btn = &tp1] {btn->OnStateChanged();  };
	tp1.setButtonText("TapeStop");
	tp1.tapestopProcessor = &audioProcessor.tapestopProcessor;
}
