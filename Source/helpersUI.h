//
// Created by blaze on 13.11.2024.
//
#pragma once
#include <JuceHeader.h>
#include "helpers.h"

class customKnob : public juce::Component, public juce::Slider::Listener
{
public:
    customKnob::customKnob(juce::String&& labelText, int&& suff, bool isInt)
    {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        label.setFont(juce::Font(juce::FontOptions("Montserrat",14.0f,0)));
        label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
        this->labelText = labelText;
        integer=isInt;
        suffixNumber = suff;
        label.setText(labelText,juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredTop);
        slider.addListener(this);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
        slider.addListener(this);

        setPrefix();
    }
    customKnob::~customKnob() override= default;

    void paint(juce::Graphics& g) override
    {

    }

    void resized() override
    {
        label.setText(labelText,juce::dontSendNotification);
        juce::Rectangle<int> area = getLocalBounds();
        juce::Rectangle<int> labelArea = area.removeFromBottom(15);
        label.setBounds(labelArea.withSizeKeepingCentre(area.getWidth(), 15));
        slider.setBounds(area);
    }

    void sliderValueChanged(juce::Slider*) override
    {
        if(integer)
            sliderValue= juce::String((int)slider.getValue())+suffix;
        if(!integer)
            sliderValue= juce::String(roundTwoDec(slider.getValue()))+suffix;
        label.setText(sliderValue,juce::dontSendNotification);
    }

    void sliderDragStarted(juce::Slider*slider) override
    {
        label.setText(sliderValue,juce::dontSendNotification);
    }

    void sliderDragEnded(juce::Slider*slider) override
    {
        label.setText(labelText,juce::dontSendNotification);
    }
    void setPrefix()
    {
        if(suffixNumber==0)
                suffix="";
        if(suffixNumber==1)
                suffix ="Hz";
        if(suffixNumber==2)
            suffix ="%";
        if(suffixNumber==3)
            suffix ="ct";
    }

    juce::Slider slider;
private:
    juce::String labelText;
    juce::String sliderValue;
    juce::Label label;
    int suffixNumber;
    juce::String suffix;
    bool integer;


};
class customSlider : public juce::Component, public juce::Slider::Listener
{
public:
    customSlider::customSlider(juce::String&& labelText, int&& suff,bool isInt, bool horizontal)
    {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        label.setFont(juce::Font(juce::FontOptions("Montserrat",14.0f,0)));
        label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
        this->labelText = labelText;
        suffixNumber = suff;
        integer = isInt;
        label.setText(labelText,juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredTop);
        slider.addListener(this);
        if(horizontal)
        {
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
        }
        if(!horizontal)
        {
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
        }


        setPrefix();
    }
    customSlider::~customSlider() override= default;

    void paint(juce::Graphics& g) override
    {

    }

    void resized() override
    {
        label.setText(labelText,juce::dontSendNotification);
        juce::Rectangle<int> area = getLocalBounds();
        juce::Rectangle<int> labelArea = area.removeFromBottom(15);
        label.setBounds(labelArea.withSizeKeepingCentre(area.getWidth(), 15));
        slider.setBounds(area);
    }

    void sliderValueChanged(juce::Slider*) override
    {
        if(integer)
            sliderValue= juce::String((int)slider.getValue())+suffix;
        if(!integer)
            sliderValue= juce::String(roundTwoDec(slider.getValue()))+suffix;
        label.setText(sliderValue,juce::dontSendNotification);
    }

    void sliderDragStarted(juce::Slider*slider) override
    {
        label.setText(sliderValue,juce::dontSendNotification);
    }

    void sliderDragEnded(juce::Slider*slider) override
    {
        label.setText(labelText,juce::dontSendNotification);
    }
    void setPrefix()
    {
        if(suffixNumber==0)
                suffix="";
        if(suffixNumber==1)
                suffix ="Hz";
        if(suffixNumber==2)
            suffix ="%";
        if(suffixNumber==3)
            suffix ="ct";
    }

    juce::Slider slider;
private:
    juce::String labelText;
    juce::String sliderValue;
    juce::Label label;
    int suffixNumber;
    bool integer;
    juce::String suffix;

};
