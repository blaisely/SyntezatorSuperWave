//
// Created by blaze on 07.11.2024.
//

#ifndef MODMATRIXGUI_H
#define MODMATRIXGUI_H
#include <JuceHeader.h>
#include "PluginProcessor.h"
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
class modMatrixGUI  : public juce::Component, private juce::ComboBox::Listener, private juce::Slider::Listener, private juce::Button::Listener
{
public:
    template<typename T>
    void addItemToFlexBox(juce::FlexBox& fb,T& item,const int& w, const int& h,const int& margin);
    modMatrixGUI(SimpleSynthAudioProcessor&);
    ~modMatrixGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* box) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void makeKnob(juce::Slider& slider,juce::Label& label);
    void makeSlider(juce::Slider& slider);
    void setUpComboBox(juce::ComboBox& box,juce::StringArray& s);

    void setUpLayout(juce::FlexBox& box,juce::ComboBox& item1,juce::Slider& item2,juce::ComboBox& item3);

private:
    const int itemWidth = 100;
    const int itemHeight = 40;
    const int margin = 5;
    juce::ComboBox source1;
    juce::ComboBox source2;
    juce::ComboBox source3;
    juce::ComboBox source4;
    juce::ComboBox source5;
    juce::ComboBox dest1;
    juce::ComboBox dest2;
    juce::ComboBox dest3;
    juce::ComboBox dest4;
    juce::ComboBox dest5;
    juce::Slider intens1;
    juce::Slider intens2;
    juce::Slider intens3;
    juce::Slider intens4;
    juce::Slider intens5;

    std::unique_ptr<ComboBoxAttachment> source1Attach;
    std::unique_ptr<ComboBoxAttachment> source2Attach;
    std::unique_ptr<ComboBoxAttachment> source3Attach;
    std::unique_ptr<ComboBoxAttachment> source4Attach;
    std::unique_ptr<ComboBoxAttachment> source5Attach;
    std::unique_ptr<ComboBoxAttachment> dest1Attach;
    std::unique_ptr<ComboBoxAttachment> dest2Attach;
    std::unique_ptr<ComboBoxAttachment> dest3Attach;
    std::unique_ptr<ComboBoxAttachment> dest4Attach;
    std::unique_ptr<ComboBoxAttachment> dest5Attach;

    std::unique_ptr<SliderAttachment> intens1Attach;
    std::unique_ptr<SliderAttachment> intens2Attach;
    std::unique_ptr<SliderAttachment> intens3Attach;
    std::unique_ptr<SliderAttachment> intens4Attach;
    std::unique_ptr<SliderAttachment> intens5Attach;
    juce::StringArray destinations{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"};
    juce::StringArray sources{"LFO 1","LFO2","LFO3","AMP","Env1","Env2"};
    SimpleSynthAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (modMatrixGUI)
};
template <typename T>
void modMatrixGUI::addItemToFlexBox(juce::FlexBox& fb, T& item, const int& w, const int& h, const int& m)
{
    fb.items.add(juce::FlexItem(item).withMaxHeight(h).withMaxWidth(w).withMargin(m).withFlex(1));
}



#endif //MODMATRIXGUI_H
