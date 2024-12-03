
#include "presetGUI.h"
#include <JuceHeader.h>


typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
presetGUI::presetGUI(SuperWaveSynthAudioProcessor& p) : audioProcessor(p), gain("GAIN",2,false,true)
{
    addAndMakeVisible(gain);
    addAndMakeVisible(init);
    init.setButtonText("INIT");
    init.setClickingTogglesState(true);
    init.addListener(this);
    gainAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"gainOVR",gain.slider);
    initAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"reset",init);
    setSize(530, 50);

    init.setLookAndFeel(&customLook);
}

presetGUI::~presetGUI()
{
    init.setLookAndFeel(nullptr);
}

void presetGUI::paint (juce::Graphics& g)
{

}

void presetGUI::resized()
{

    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> gainSection = area.removeFromRight(150);
    juce::Rectangle<int> leftSection = area.removeFromLeft(50);

    gain.setBounds(gainSection);
    init.setBounds(leftSection);

}

void presetGUI::timerCallback()
{

}

void presetGUI::comboBoxChanged(juce::ComboBox* box)
{
}

void presetGUI::sliderValueChanged(juce::Slider* slider)
{
}

void presetGUI::buttonClicked(juce::Button* button)
{
    if(button==&init)
    {

    }
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



