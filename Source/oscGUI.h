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
    explicit oscGUI(SimpleSynthAudioProcessor&);
    ~oscGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox*) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void makeKnob(juce::Slider& slider, juce::String ID, juce::Label& label);
    void makeSlider(juce::Slider& slider,juce::Label& label);

private:
    //juce::novationDial dial;
    customLookAndFeel customLook;
    juce::Label Gain_osc1_label{ "GAIN","GAIN" };
    juce::Label panOsc1Label{ "Pan","PAN" };
    juce::Label panOsc2Label{ "Pan","PAN" };
    juce::Label Gain_osc2_label{ "Gain","GAIN" };
    juce::Label OSC1{"OSC1","OSC1"};
    juce::Label waveTypeLabel1{"WAVE","WAVE"};
    juce::Label PWMLabel1{"PW","PW"};
    juce::Label waveTypeLabel2{"WAVE","WAVE"};
    juce::Label PWMLabel2{"PW","PW"};
    juce::Label OSC2{"OSC2","OSC2"};
    juce::Label detuneLabel{ "DETUNE","DETUNE" };
    juce::Label detuneMixLabel{ "MIX", "MIX" };
    juce::Label octave_osc1_label{ "OCTAVE","OCTAVE" };
    juce::Label octave_osc2_label{ "OCTAVE","OCTAVE" };
    juce::Label coarse_osc1_label{ "COARSE","COARSE" };
    juce::Label semiDetune_osc1_label{ "semiDet","SEMI" };
    juce::Label coarse_osc2_label{ "COARSE","COARSE" };
    juce::Label semiDetune_osc2_label{ "semiDet","SEMI" };
    juce::ComboBox oscMenu_osc1;
    juce::ComboBox oscMenu_osc2;
    juce::ComboBox preset;
    juce::Slider Gain_osc1;
    juce::Slider Gain_osc2;
    juce::Slider detune;
    juce::Slider detuneMix;
    juce::Slider octave_osc1;
    juce::Slider octave_osc2;
    juce::Slider coarse_osc1;
    juce::Slider semiDetune_osc1;
    juce::Slider coarse_osc2;
    juce::Slider semiDetune_osc2;
    juce::Slider panOsc1;
    juce::Slider panOsc2;
    juce::Slider crush;
    juce::Slider waveTypeOSC1;
    juce::Slider PWOSC1;
    juce::Slider waveTypeOSC2;
    juce::Slider PWOSC2;
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
    SimpleSynthAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (oscGUI)
};

template <typename T>
void oscGUI::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}
