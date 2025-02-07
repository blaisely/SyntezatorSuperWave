//
// Created by blaze on 07.11.2024.
//

#ifndef PRESETGUI_H
#define PRESETGUI_H
#include <JuceHeader.h>

#include "customLook.h"
#include "helpersUI.h"
#include "../PluginProcessor.h"
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
class presetGUI  : public juce::Component, private juce::ComboBox::Listener, private juce::Slider::Listener, private juce::Button::Listener, public juce::Timer
{
public:
    template<typename T>
    void addItemToFlexBox(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    explicit presetGUI(SuperWaveSynthAudioProcessor&);
    ~presetGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* box) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void makeKnob(juce::Slider& slider,juce::Label& label);
    void makeSlider(juce::Slider& slider,juce::Label& label);

private:
    customSlider gain;
    customSlider stereo;
    customLookAndFeel customLook;
    juce::TextButton init;
    juce::Label gainLabel{"Gain","GAIN"};
    juce::Label stereoLabel{"Stereo","STEREO"};
    std::unique_ptr<SliderAttachment> gainAttach;
    std::unique_ptr<SliderAttachment> stereoAttach;
    std::unique_ptr<ButtonAttachment> initAttach;
    SuperWaveSynthAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (presetGUI)
};
template <typename T>
void presetGUI::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}
#endif //PRESETGUI_H
