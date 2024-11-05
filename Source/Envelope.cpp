/*
  ==============================================================================

    Envelope.cpp
    Created: 14 Apr 2024 7:02:37pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>

#include <memory>
#include "Envelope.h"

//==============================================================================
Envelope::Envelope(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
   makeSlider(attackAmp,attackAmpLabel);
   makeSlider(decayAmp,decayAmpLabel);
   makeSlider(sustainAmp,sustainAmpLabel);
   makeSlider(releaseAmp,releaseAmpLabel);
    makeSlider(attackMod,attackModLabel);
    makeSlider(decayMod,decayModLabel);
    makeSlider(sustainMod,sustainModLabel);
    makeSlider(releaseMod,releaseModLabel);
    makeSlider(modAmount,modAmountLabel);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "attack", attackAmp);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "decay", decayAmp);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "sustain", sustainAmp);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "release", releaseAmp);

    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"attackOsc2",attackMod);
    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"decayOsc2",decayMod);
    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"sustainOsc2",sustainMod);
    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"releaseOsc2",releaseMod);
    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"filterEnvelope",modAmount);

    addAndMakeVisible(modEnvType);
    modEnvType.setToggleable(true);
    modEnvType.setButtonText("Env 1");
    modEnvType.addListener(this);

    setSize(530, 150);
}

Envelope::~Envelope()
= default;

void Envelope::paint (juce::Graphics& g)
{
    juce::Rectangle<int> area = getLocalBounds();
    g.setColour(juce::Colour(0xff312F2F));
    g.fillRoundedRectangle(area.toFloat(),6);
   
}

void Envelope::resized()
{
    constexpr int sliderWidth = 25;
    constexpr int sliderHeight = 100;
    constexpr int margin  = 1;
    constexpr int labelWidth = 25;
    constexpr int labelHeight = 15;
    constexpr int buttonWidth = 40;
    constexpr int buttonHeight = 20;
    constexpr int amountSliderWidth = 20;
    constexpr int amountSliderHeight = 80;
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> ampArea = area.removeFromLeft(122).reduced(5);
    juce::Rectangle<int> ampLabelArea = ampArea.removeFromBottom(15);
    juce::Rectangle<int> modEnvelope = area.removeFromLeft(122).reduced(5);
    juce::Rectangle<int> modLabelArea = modEnvelope.removeFromBottom(15);
    juce::Rectangle<int> envelopeButtonsArea = area.removeFromLeft(40).reduced(5);
    juce::Rectangle<int> envelopeButtonsLabelArea = envelopeButtonsArea.removeFromBottom(15);
    juce::Rectangle<int> lfoArea = area.removeFromLeft(122).expanded(5);
    juce::Rectangle<int> controlsArea = area.removeFromLeft(122);

    juce::FlexBox amp;
    amp.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(amp,attackAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,decayAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,sustainAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,releaseAmp,sliderWidth,sliderHeight,margin);
    amp.performLayout(ampArea);

    juce::FlexBox ampLabels;
    ampLabels.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(ampLabels,attackAmpLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(ampLabels,decayAmpLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(ampLabels,sustainAmpLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(ampLabels,releaseAmpLabel,labelWidth,labelHeight,margin);
    ampLabels.performLayout(ampLabelArea);

    juce::FlexBox mod;
    mod.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(mod,attackMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,decayMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,sustainMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,releaseMod,sliderWidth,sliderHeight,margin);
    mod.performLayout(modEnvelope);

    juce::FlexBox modLabels;
    modLabels.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(modLabels,attackModLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(modLabels,decayModLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(modLabels,sustainModLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(modLabels,releaseModLabel,labelWidth,labelHeight,margin);
    modLabels.performLayout(modLabelArea);

    juce::FlexBox modControls;
    modControls.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(modControls,modEnvType,buttonWidth,buttonHeight,margin);
    addItemToFlexBox(modControls,modAmount,amountSliderWidth,amountSliderHeight,margin);
    modControls.performLayout(envelopeButtonsArea);
    modAmountLabel.setBounds(envelopeButtonsLabelArea);



}

void Envelope::sliderValueChanged(juce::Slider* slider)
{
}

void Envelope::buttonStateChanged(juce::Button* button)
{

}

void Envelope::buttonClicked(juce::Button* button)
{
    bool state = true;
    if(button==&modEnvType)
    {
        if(!button->getToggleState())
            button->setButtonText("Env 2");
        else if(button->getToggleState())
            button->setButtonText("Env 1");
        state= button->getToggleState();
        button->setToggleState(!state,juce::dontSendNotification);

    }
}

void Envelope::makeKnob(juce::Slider& slider,juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void Envelope::makeSlider(juce::Slider& slider, juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}
