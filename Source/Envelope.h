/*
  ==============================================================================

    Envelope.h
    Created: 14 Apr 2024 7:02:37pm
    Author:  blaze

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
class Envelope  : public juce::Component, private juce::Slider::Listener, public juce::Button::Listener
{
public:
    template<typename T>
   void addItemToFlexBox(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    Envelope(SimpleSynthAudioProcessor&);
    ~Envelope() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider);
    void buttonStateChanged(juce::Button*) override;
    void buttonClicked(juce::Button*) override;
    void makeKnob(juce::Slider& slider, juce::Label& label);
    void makeSlider(juce::Slider& slider,juce::Label& label);
    

private:
    juce::TextButton modEnvType;
    juce::Slider attackAmp;
    juce::Slider attackMod;
    juce::Slider decayAmp;
    juce::Slider decayMod;
    juce::Slider sustainAmp;
    juce::Slider sustainMod;
    juce::Slider releaseAmp;
    juce::Slider releaseMod;
    juce::Slider modAmount;
    juce::Label attackAmpLabel{"Attack","A"};
    juce::Label attackModLabel{"Attack","A"};
    juce::Label decayAmpLabel{"Decay","D"};
    juce::Label decayModLabel{"Decay","D"};
    juce::Label sustainAmpLabel{"Sustain","S"};
    juce::Label sustainModLabel{"Sustain","S"};
    juce::Label releaseAmpLabel{"Release","R"};
    juce::Label releaseModLabel{"Release","R"};
    juce::Label modAmountLabel{"amt","Amount"};
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> attackModAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> decayModAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> sustainModAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> releaseModAttachment;
    std::unique_ptr<SliderAttachment> modAmountAttach;


    SimpleSynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Envelope)
};

template <typename T>
void Envelope::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}
