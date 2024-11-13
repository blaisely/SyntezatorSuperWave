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
filterGUI::filterGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p),filterCutOff("CutOff",1,true),filterResonance("Resonance",2,true),
filterDrive("Drive",2,true),keyTrackOffset("Offset",0,true,true)
{
    addAndMakeVisible(filterCutOff);
    addAndMakeVisible(filterResonance);
    addAndMakeVisible(filterDrive);
    addAndMakeVisible(keyTrackOffset);
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

    filterCutOffAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterCutoff",filterCutOff.slider);
    filterResonanceAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterRes",filterResonance.slider);
    filterDriveAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterDrive",filterDrive.slider);
    keyTrackOffsetAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"filterKeytrackOffset",keyTrackOffset.slider);
    filterKeytrackAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"filterKeytrackEnable",filterKeytracking);
    filterEmuAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"filterbutton",filterEmu);
    filterTypeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"filterType",filterType);


    setSize(320, 220);

    filterCutOff.slider.setLookAndFeel(&customLook);
    filterResonance.slider.setLookAndFeel(&customLook);
    filterDrive.slider.setLookAndFeel(&customLook);
    filterEmu.setLookAndFeel(&emuLook);
    filterKeytracking.setLookAndFeel(&customLook);
    filterLabel.setLookAndFeel(&labelLook);
}

filterGUI::~filterGUI()
{
    filterCutOff.slider.setLookAndFeel(nullptr);
    filterResonance.setLookAndFeel(nullptr);
    filterDrive.setLookAndFeel(nullptr);
    filterEmu.setLookAndFeel(nullptr);
    filterKeytracking.setLookAndFeel(nullptr);
    filterLabel.setLookAndFeel(nullptr);
}

void filterGUI::paint (juce::Graphics& g)
{
}

void filterGUI::resized()
{
    constexpr int knobSize = 90;
    constexpr int margin =1;
    constexpr int buttonMargin = 5;
    constexpr int labelWidth = 80;
    constexpr int labelHeight = 15;
    constexpr int buttonWidth = 50;
    constexpr int buttonHeight = 20;
    constexpr int horizontalSliderHeight = 30;
    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> titleArea = area.removeFromTop(40).reduced(5);
    juce::Rectangle<int> filterSelection = area.removeFromTop(30);
    juce::Rectangle<int> leftSection = area.removeFromLeft(110);
    juce::Rectangle<int> rightSection = area.removeFromLeft(110);
    filterLabel.setBounds(titleArea.withSizeKeepingCentre(65,40));
    filterType.setBounds(filterSelection.withSizeKeepingCentre(80,30));
    juce::FlexBox left;
    left.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(left,filterCutOff,knobSize,knobSize,margin);
    addItemToFlexBox(left,filterResonance,knobSize,knobSize,margin);
    left.performLayout(leftSection);

    juce::FlexBox right;
    right.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(right,filterDrive,knobSize,knobSize,margin);

    addItemToFlexBox(right,filterEmu,buttonWidth,buttonHeight,buttonMargin);
    addItemToFlexBox(right,filterKeytracking,buttonWidth,buttonHeight,buttonMargin);
    addItemToFlexBox(right,keyTrackOffset,knobSize,horizontalSliderHeight,margin);
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


