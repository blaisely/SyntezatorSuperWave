/*
  ==============================================================================

    filterGUI.h
    Created: 13 Apr 2024 3:03:55pm
    Author:  blaze

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "customLook.h"
#include "PluginProcessor.h"
#include "helpersUI.h"

//==============================================================================


typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
class filterGUI  : public juce::Component, private juce::ComboBox::Listener, private juce::Slider::Listener, private juce::Button::Listener
{
public:
    template<typename T>
    void addItemToFlexBox(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    filterGUI(SimpleSynthAudioProcessor&);
    ~filterGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* box) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
private:
    customLookAndFeel customLook;
    notToggleButtonLook emuLook;
    filterLabel labelLook;

    customKnob filterCutOff;
    customKnob filterResonance;
    customKnob filterDrive;
    juce::TextButton filterKeytracking;
    customSlider keyTrackOffset;
    juce::Label filterLabel{"FILTER","FILTER"};
    std::unique_ptr<SliderAttachment> filterCutOffAttach;
    std::unique_ptr<SliderAttachment> filterResonanceAttach;
    std::unique_ptr<SliderAttachment> filterDriveAttach;
    std::unique_ptr<SliderAttachment> keyTrackOffsetAttach;
    std::unique_ptr<ButtonAttachment> filterEmuAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> filterKeytrackAttach;
    std::unique_ptr<ComboBoxAttachment> filterTypeAttachment;
    juce::TextButton filterEmu;
    juce::ComboBox filterType;
    SimpleSynthAudioProcessor& audioProcessor;
    juce::StringArray filterTypes{"LPF","HPF","BPF"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (filterGUI)
};
template <typename T>
void filterGUI::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}


