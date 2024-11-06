/*
  ==============================================================================

    filterGUI.cpp
    Created: 13 Apr 2024 3:03:55pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>
#include "filterGUI.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
filterGUI::filterGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    makeKnob(filterCutOff,filterCutOffLabel);
    makeKnob(filterResonance,filterResonanceLabel);
    makeKnob(filterDrive,filterDriveLabel);
    makeSlider(keyTrackOffset,offsetLabel);
    filterCutOffAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterCutoff",filterCutOff);
    filterResonanceAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterRes",filterResonance);
    filterDriveAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterDrive",filterDrive);
    keyTrackOffsetAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterKeytrackOffset",keyTrackOffset);
    filterKeytrackAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"filterKeytrackEnable",filterKeytracking);
    filterEmuAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"filterbutton",filterEmu);
    addAndMakeVisible(filterEmu);
    filterEmu.addListener(this);
    filterEmu.setButtonText("SVF");
    filterEmu.setToggleable(true);
    addAndMakeVisible(filterKeytracking);
    filterKeytracking.addListener(this);
    filterKeytracking.setButtonText("KeyTrack");
    filterKeytracking.setToggleable(true);
    addAndMakeVisible(filterLabel);

    setSize(320, 275);
}

filterGUI::~filterGUI()
{
}

void filterGUI::paint (juce::Graphics& g)
{
}

void filterGUI::resized()
{
    constexpr int knobSize = 100;
    constexpr int margin =1;
    constexpr int labelWidth = 100;
    constexpr int labelHeight = 15;
    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> titleArea = area.removeFromTop(30).reduced(5);
    juce::Rectangle<int> leftSection = area.removeFromLeft(115);
    filterLabel.setBounds(titleArea);
    juce::FlexBox left;
    left.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(left,filterCutOff,knobSize,knobSize,margin);
    addItemToFlexBox(left,filterCutOffLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(left,filterResonance,knobSize,knobSize,margin);
    addItemToFlexBox(left,filterResonanceLabel,labelWidth,labelHeight,margin);
    left.performLayout(leftSection);

}

void filterGUI::comboBoxChanged(juce::ComboBox* box)
{
}

void filterGUI::sliderValueChanged(juce::Slider* slider)
{
}

void filterGUI::buttonClicked(juce::Button* button)
{

}

void filterGUI::makeKnob(juce::Slider& slider,juce::Label& label)
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

void filterGUI::makeSlider(juce::Slider& slider, juce::Label& label)
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

