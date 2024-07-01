/*
  ==============================================================================

    filterGUI.h
    Created: 13 Apr 2024 3:03:55pm
    Author:  blaze

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================


class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel() {}

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool isMouseOverButton, bool isButtonDown) override
    {
        juce::Font font(10.0f); // Set your desired font size here
        g.setFont(font);
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
            : juce::TextButton::textColourOffId));

        const int yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
        const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        const int fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
        const int leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (isMouseOverButton || isButtonDown ? 2 : 4));
        const int rightIndent = juce::jmin(fontHeight, 2 + (cornerSize + fontHeight) / (isMouseOverButton || isButtonDown ? 2 : 4));

        const int textWidth = button.getWidth() - leftIndent - rightIndent;

        const juce::String text(button.getButtonText());

        if (textWidth > 0)
            g.drawFittedText(text, leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                juce::Justification::centred, 2);
    }
};

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
class filterGUI  : public juce::Component, private juce::ComboBox::Listener, private juce::Slider::Listener, private juce::Button::Listener
{
public:
    filterGUI(SimpleSynthAudioProcessor&);
    ~filterGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* box) override;

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void makeSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::String ID, juce::Label& label,float initValue,float maxValue,float minValue);

private:
    juce::Slider cutOffSlider;
    juce::Slider filterResonance;
    juce::Slider lfoFreq;
    juce::Slider lfoDepth;
    juce::TextButton filterOn;
    juce::Label lfoFreqLabel = { "LFO FREQ", "LFO FREQ" };
    juce::Label lfoDepthLabel = { "LFO DEPTH", "LFO DEPTH" };
    SimpleSynthAudioProcessor& audioProcessor;
    juce::String ID_lfoFreq = {"lfofreq"};
    juce::String ID_lfoDepth = { "lfodepth" };
    std::unique_ptr<SliderAttachment> filterCutOffAttach;
    std::unique_ptr<SliderAttachment> filterResonanceAttach;
    std::unique_ptr<SliderAttachment> lfoFreqAttach;
    std::unique_ptr<SliderAttachment> lfoDepthAttach;
   
    CustomLookAndFeel buttonLook;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterSelection;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttach;
    juce::ComboBox filterMenu;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (filterGUI)
};
