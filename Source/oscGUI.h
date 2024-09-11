/*
  ==============================================================================

    oscGUI.h
    Created: 13 Apr 2024 2:50:39pm
    Author:  blaze

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SharedData.h"

//==============================================================================
/*
*/
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
class oscGUI  : public juce::Component,
    private juce::ComboBox::Listener, private juce::Slider::Listener
{
public:
    oscGUI(SimpleSynthAudioProcessor&);
    ~oscGUI() override;

    void paint (juce::Graphics&) override;
    void setUpLabels(juce::FlexBox& labelsTop,juce::Label& firstLabel,juce::Label& secondLabel);
    void setUpKnobs(juce::FlexBox& flex,juce::Slider& firstSlider,juce::Slider& secondSlider);
    void resized() override;
    void setUpFlex(juce::FlexBox& flex);
    void comboBoxChanged(juce::ComboBox*) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void makeSlider(juce::Slider& slider, juce::String ID, juce::Label& label);
private:
  
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1Selection;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc2Selection;

    juce::Label Gain_osc1_label{ "Gain OSC1","Gain OSC1" };
    juce::Label panOsc1Label{ "Pan","Pan" };
    juce::Label panOsc2Label{ "Pan","Pan" };
    juce::Label Gain_osc2_label{ "Gain OSC2","Gain OSC2" };

    juce::Label detuneLabel{ "DETUNE","DETUNE" };
    juce::Label volumeLabel{ "VOLUME", "VOLUME" };
    juce::Label octave_osc1_label{ "OCTAVE","OCTAVE" };
    juce::Label octave_osc2_label{ "OCTAVE","OCTAVE" };
    juce::Label coarse_osc1_label{ "COARSE","COARSE" };
    juce::Label coarse_osc2_label{ "COARSE","COARSE" };
    juce::ComboBox oscMenu_osc1;
    juce::ComboBox oscMenu_osc2;
    juce::ComboBox preset;
    juce::Slider Gain_osc1;
    juce::Slider Gain_osc2;
    juce::Slider detune;
    juce::Slider volume;
    juce::Slider octave_osc1;
    juce::Slider octave_osc2;
    juce::Slider coarse_osc1;
    juce::Slider coarse_osc2;
    juce::Slider panOsc1;
    juce::Slider panOsc2;
    juce::Slider crush;
    std::unique_ptr<SliderAttachment> Gain_osc1Attach;
    std::unique_ptr<SliderAttachment> panOsc1Attach;
    std::unique_ptr<SliderAttachment> panOsc2Attach;
    std::unique_ptr<SliderAttachment> Gain_osc2Attach;
    std::unique_ptr<SliderAttachment> detuneAttach;
    std::unique_ptr<SliderAttachment> volumeAttach;
    std::unique_ptr<SliderAttachment> octave_osc1Attach;
    std::unique_ptr<SliderAttachment> octave_osc2Attach;
    std::unique_ptr<SliderAttachment> coarse_osc1Attach;
    std::unique_ptr<SliderAttachment> coarse_osc2Attach;
    juce::ValueTree tree;
    SimpleSynthAudioProcessor& audioProcessor;
    void setUpRightOsc(const int& width);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (oscGUI)
};
