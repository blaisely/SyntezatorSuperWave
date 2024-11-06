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
#include "SharedData.h"

//==============================================================================
/*
*/
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
class Envelope  : public juce::Component, private juce::Slider::Listener, public juce::Button::Listener, public juce::ComboBox::Listener
{
public:
    template<typename T>
   void addItemToFlexBox(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    template<typename T>
   void addItemToFlexBoxWithAlign(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    Envelope(SimpleSynthAudioProcessor&);
    ~Envelope() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider);
    void buttonStateChanged(juce::Button*) override;
    void buttonClicked(juce::Button*) override;
    void makeKnob(juce::Slider& slider, juce::Label& label);
    void makeSlider(juce::Slider& slider,juce::Label& label);
    void setUpLFOAttachments(juce::StringArray& id);
    void setUpLFOKnobs(juce::StringArray& id);
    void setUpModAttachments(juce::StringArray& id);
    void setUpModSliders(juce::StringArray& id);
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void changeEnvelopes(int envelope);
    

private:
    juce::TextButton modEnvType;
    juce::TextButton loopEnvelope;
    juce::TextButton lfoUnipolar;
    juce::TextButton lfoReset;
    juce::Slider attackAmp;
    juce::Slider attackMod;
    juce::Slider decayAmp;
    juce::Slider decayMod;
    juce::Slider sustainAmp;
    juce::Slider sustainMod;
    juce::Slider releaseAmp;
    juce::Slider releaseMod;
    juce::Slider modAmount;
    juce::Slider lfoDepth;
    juce::Slider lfoFreq;
    juce::ComboBox lfoType;
    juce::ComboBox lfoNumber;
    juce::Label attackAmpLabel{"Attack","A"};
    juce::Label attackModLabel{"Attack","A"};
    juce::Label decayAmpLabel{"Decay","D"};
    juce::Label decayModLabel{"Decay","D"};
    juce::Label sustainAmpLabel{"Sustain","S"};
    juce::Label sustainModLabel{"Sustain","S"};
    juce::Label releaseAmpLabel{"Release","R"};
    juce::Label releaseModLabel{"Release","R"};
    juce::Label modAmountLabel{"amt","Amount"};
    juce::Label lfoDepthLabel{"Depth", "Depth"};
    juce::Label lfoFreqLabel{"Freq", "Freq"};
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> attackModAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> decayModAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> sustainModAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> releaseModAttachment;
    std::unique_ptr<SliderAttachment> modAmountAttach;
    std::unique_ptr<SliderAttachment> lfoDepthAttach;
    std::unique_ptr<SliderAttachment> lfoFreqAttach;
    std::unique_ptr<ComboBoxAttachment> lfoTypeAttach;
    std::unique_ptr<ButtonAttachment> loopEnvelopeAttach;
    std::unique_ptr<ButtonAttachment> lfoUnipolarAttach;
    std::unique_ptr<ButtonAttachment> lfoResetAttach;

    juce::StringArray envelope2IDs{"attackOsc2","decayOsc2","sustainOsc2","releaseOsc2","filterEnvelope","loopEnvelope"};
    juce::StringArray envelope3IDs{"attackOsc3","decayOsc3","sustainOsc3","releaseOsc3","filterEnvelope2","loopEnvelope2"};
    juce::StringArray LFO1IDs{"lfodepth","lfofreq","lfo1Unipolar","lfoReset","lfoType"};
    juce::StringArray LFO2IDs{"lfo2depth","lfo2freq","lfo2Unipolar","lfoReset2","lfo2Type"};
    juce::StringArray LFO3IDs{"lfo3depth","lfo3freq","lfo3Unipolar","lfoReset3","lfo3Type"};

    SimpleSynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Envelope)
};

template <typename T>
void Envelope::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}
template <typename T>
void Envelope::addItemToFlexBoxWithAlign(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1).withAlignSelf(juce::FlexItem::AlignSelf::autoAlign));
}
