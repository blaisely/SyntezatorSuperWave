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
#include "customLook.h"
#include "SharedData.h"
#include "helpersUI.h"

//==============================================================================
/*
*/
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
class oscGUI  : public juce::Component,
    private juce::ComboBox::Listener, private juce::Slider::Listener
{
public:
    template<typename T>
    void addItemToFlexBox(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    explicit oscGUI(SuperWaveSynthAudioProcessor&);
    ~oscGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox*) override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    //juce::novationDial dial;
    customLookAndFeel customLook;

    juce::Label OSC1{"OSC1","OSC1"};

    juce::Label OSC2{"OSC2","OSC2"};

    juce::ComboBox oscMenu_osc1;
    juce::ComboBox oscMenu_osc2;
    juce::ComboBox preset;
    customSlider Gain_osc1;
    customSlider Gain_osc2;
    customKnob detune;
    customKnob detuneMix;
    customKnob octave_osc1;
    customKnob octave_osc2;
    customKnob coarse_osc1;
    customKnob semiDetune_osc1;
    customKnob coarse_osc2;
    customKnob semiDetune_osc2;
    customSlider panOsc1;
    customSlider panOsc2;
    customKnob waveTypeOSC1;
    customKnob PWOSC1;
    customKnob waveTypeOSC2;
    customKnob PWOSC2;
    std::unique_ptr<SliderAttachment> Gain_osc1Attach;
    std::unique_ptr<SliderAttachment> panOsc1Attach;
    std::unique_ptr<SliderAttachment> panOsc2Attach;
    std::unique_ptr<SliderAttachment> Gain_osc2Attach;
    std::unique_ptr<SliderAttachment> detuneAttach;
    std::unique_ptr<SliderAttachment> volumeAttach;
    std::unique_ptr<SliderAttachment> octave_osc1Attach;
    std::unique_ptr<SliderAttachment> octave_osc2Attach;
    std::unique_ptr<SliderAttachment> coarse_osc1Attach;
    std::unique_ptr<SliderAttachment> semiDetune_osc1Attach;
    std::unique_ptr<SliderAttachment> coarse_osc2Attach;
    std::unique_ptr<SliderAttachment> semiDetune_osc2Attach;
    std::unique_ptr<SliderAttachment> waveTypeOSC1Attach;
    std::unique_ptr<SliderAttachment> PWOSC1Attach;
    std::unique_ptr<SliderAttachment> waveTypeOSC2Attach;
    std::unique_ptr<SliderAttachment> PWOSC2Attach;
    juce::ValueTree tree;
    SuperWaveSynthAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (oscGUI)
};

template <typename T>
void oscGUI::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}
