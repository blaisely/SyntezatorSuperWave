
#include "presetGUI.h"
#include <JuceHeader.h>


typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
presetGUI::presetGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    makeSlider(gain,gainLabel);
    gainAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"gainOVR",gain);
    setSize(530, 50);
}

presetGUI::~presetGUI()
{

}

void presetGUI::paint (juce::Graphics& g)
{

}

void presetGUI::resized()
{

    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> labelSection = area.removeFromRight(50);
    juce::Rectangle<int> gainSection = area.removeFromRight(150);

    gainLabel.setBounds(labelSection);
    gain.setBounds(gainSection);

}

void presetGUI::comboBoxChanged(juce::ComboBox* box)
{
}

void presetGUI::sliderValueChanged(juce::Slider* slider)
{
}

void presetGUI::buttonClicked(juce::Button* button)
{

}

void presetGUI::makeKnob(juce::Slider& slider,juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(12.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void presetGUI::makeSlider(juce::Slider& slider,juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(12.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}



