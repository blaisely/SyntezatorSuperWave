/*
  ==============================================================================

    Envelope.cpp
    Created: 14 Apr 2024 7:02:37pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Envelope.h"

//==============================================================================
Envelope::Envelope(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    attack.setRange(0.1f, 10.0f, 0.01f);
    decay.setRange(0.1f, 15.0f, 0.01f);
    sustain.setRange(0.1f, 1.0f, 0.01f);
    release.setRange(0.1f, 10.0f, 0.01f);

    attack.setValue(0.1f);
    decay.setValue(6.0f);
    sustain.setValue(1.0f);
    release.setValue(.5f);

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

    attackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "attack", attack));
    decayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "decay", decay));
    sustainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "sustain", sustain));
    releaseAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "release", release));

    

    setSize(200, 200);
}

Envelope::~Envelope()
{
}

void Envelope::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::blueviolet);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

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
