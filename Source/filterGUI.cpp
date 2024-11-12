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
    addAndMakeVisible(filterEmu);
    filterEmu.addListener(this);
    filterEmu.setButtonText("SVF");
    filterEmu.setToggleable(true);
    filterEmu.setClickingTogglesState(true);
    addAndMakeVisible(filterKeytracking);
    filterKeytracking.addListener(this);
    filterKeytracking.setButtonText("KeyTrack");
    filterKeytracking.setClickingTogglesState(true);
    filterKeytracking.setToggleable(true);
    addAndMakeVisible(filterLabel);
    addAndMakeVisible(filterType);
    filterType.addItemList(filterTypes,1);

    filterCutOffAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterCutoff",filterCutOff);
    filterResonanceAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterRes",filterResonance);
    filterDriveAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterDrive",filterDrive);
    keyTrackOffsetAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterKeytrackOffset",keyTrackOffset);
    filterKeytrackAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"filterKeytrackEnable",filterKeytracking);
    filterEmuAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"filterbutton",filterEmu);
    filterTypeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"filterType",filterType);


    setSize(320, 220);

    filterCutOff.setLookAndFeel(&dial);
    filterResonance.setLookAndFeel(&dial);
    filterDrive.setLookAndFeel(&dial);
}

filterGUI::~filterGUI()
{
    filterCutOff.setLookAndFeel(nullptr);
    filterResonance.setLookAndFeel(nullptr);
    filterDrive.setLookAndFeel(nullptr);
}

void filterGUI::paint (juce::Graphics& g)
{
}

void filterGUI::resized()
{
    constexpr int knobSize = 80;
    constexpr int margin =1;
    constexpr int labelWidth = 80;
    constexpr int labelHeight = 15;
    constexpr int buttonWidth = 50;
    constexpr int buttonHeight = 20;
    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> titleArea = area.removeFromTop(20).reduced(5);
    juce::Rectangle<int> filterSelection = area.removeFromTop(30);
    juce::Rectangle<int> leftSection = area.removeFromLeft(110);
    juce::Rectangle<int> rightSection = area.removeFromLeft(110);
    filterLabel.setBounds(titleArea.withSizeKeepingCentre(65,20));
    filterType.setBounds(filterSelection.withSizeKeepingCentre(80,30));
    juce::FlexBox left;
    left.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(left,filterCutOff,knobSize,knobSize,margin);
    addItemToFlexBox(left,filterCutOffLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(left,filterResonance,knobSize,knobSize,margin);
    addItemToFlexBox(left,filterResonanceLabel,labelWidth,labelHeight,margin);
    left.performLayout(leftSection);

    juce::FlexBox right;
    right.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(right,filterDrive,knobSize,knobSize,margin);
    addItemToFlexBox(right,filterDriveLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(right,filterEmu,buttonWidth,buttonHeight,margin);
    addItemToFlexBox(right,filterKeytracking,buttonWidth,buttonHeight,margin);
    addItemToFlexBox(right,keyTrackOffset,knobSize,buttonHeight,margin);
    addItemToFlexBox(right,offsetLabel,labelWidth,labelHeight,margin);
    right.performLayout(rightSection);

}

void filterGUI::comboBoxChanged(juce::ComboBox* box)
{
}

void filterGUI::sliderValueChanged(juce::Slider* slider)
{
}

void filterGUI::buttonClicked(juce::Button* button)
{
    bool state = true;
    if(button==&filterEmu)
    {
        if(!button->getToggleState())
                button->setButtonText("Ladder");
        if(button->getToggleState())
                button->setButtonText("SVF");
    }
    if(button==&filterKeytracking)
    {

    }
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

