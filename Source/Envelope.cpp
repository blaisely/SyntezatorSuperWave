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
    attack.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
    decay.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
    sustain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
    release.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);

    attack.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    decay.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    sustain.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    release.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);


    attack.addListener(this);
    decay.addListener(this);
    sustain.addListener(this);
    release.addListener(this);

    addAndMakeVisible(attack);
    addAndMakeVisible(decay);
    addAndMakeVisible(sustain);
    addAndMakeVisible(release);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "attack", attack);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "decay", decay);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "sustain", sustain);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "release", release);

    

    setSize(200, 200);
}

Envelope::~Envelope()
= default;

void Envelope::paint (juce::Graphics& g)
{

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
   
}

void Envelope::resized()
{
    juce::Rectangle<int> area = getLocalBounds();

    attack.setBounds(area.removeFromLeft(50));
    decay.setBounds(area.removeFromLeft(50));
    sustain.setBounds(area.removeFromLeft(50));
    release.setBounds(area.removeFromLeft(50));

}

void Envelope::sliderValueChanged(juce::Slider* slider)
{
}
