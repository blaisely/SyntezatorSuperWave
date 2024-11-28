//
// Created by blaze on 13.11.2024.
//
#pragma once
#include <../../JuceLibraryCode/JuceHeader.h>
#include "../helpers.h"
#include "customLook.h"

class customKnob : public juce::Component, public juce::Slider::Listener, juce::Timer
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
        slider.setLookAndFeel(&customLook);
        setPrefix();
    }
    customKnob::~customKnob()
    {
        slider.setLookAndFeel(nullptr);
    };
    void timerCallback() override
    {
        label.setText(labelText,juce::dontSendNotification);
        stopTimer();

    }

    void paint(juce::Graphics& g) override
    {

    }

    void resized() override
    {
        label.setText(labelText,juce::dontSendNotification);
        juce::Rectangle<int> area = getLocalBounds();
        juce::Rectangle<int> labelArea = area.removeFromBottom(14);
        label.setBounds(labelArea.withSizeKeepingCentre(area.getWidth()*4, 14));
        slider.setBounds(area);
    }

    void sliderValueChanged(juce::Slider*) override
    {
        if(integer)
            sliderValue= juce::String((int)slider.getValue())+suffix;
        if(!integer)
        {
            auto maxValue = slider.getMaximum();
            float sliderV=0;
            if(maxValue<=1.f)
                sliderV = slider.getValue()*100.f;
            else
                sliderV = slider.getValue();
            sliderValue= juce::String(roundTwoDec(sliderV))+suffix;
        }
        if(waveform)
        {
            float sliderVal = slider.getValue();
            float wave1 = std::abs(static_cast<float>(std::abs(1.f-(sliderVal)))-static_cast<int>(sliderVal))*100;
            if(wave1<20)
                wave1=10;
            wave1Percent = juce::String(wave1);
            wave2Percent = juce::String(((sliderVal-(int)sliderVal)*100));
            if(sliderVal == 0.0)
                sliderValue = "SINE";
            if(sliderVal>0.0 && sliderVal<1.0)
                sliderValue = wave1Percent+"/"+wave2Percent;
            if(sliderVal ==1.0)
                sliderValue="TRIANGLE";
            if(sliderVal>1.0 && sliderVal<2.0)
                sliderValue = wave1Percent+"/"+wave2Percent;
            if(sliderVal ==2.0)
                sliderValue="SQUARE";
            if(sliderVal>2.0 && sliderVal<3.0)
                sliderValue = wave1Percent+"/"+wave2Percent;
            if(sliderVal ==3.0)
                sliderValue="SAW";

        }

        label.setText(sliderValue,juce::dontSendNotification);
        if(!dragStarted)
            startTimer(50);

    }

    void sliderDragStarted(juce::Slider*slider) override
    {
        label.setText(sliderValue,juce::dontSendNotification);
        dragStarted = true;
    }

    void sliderDragEnded(juce::Slider*slider) override
    {
        label.setText(labelText,juce::dontSendNotification);
        dragStarted = false;
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
        if(suffixNumber==4)
        {
            waveform = true;
            suffix="";
        }

    }

    juce::Slider slider;
private:
    juce::String labelText;
    juce::String sliderValue;
    juce::Label label;
    int suffixNumber;
    juce::String suffix;
    bool integer;
    juce::String wave1Percent;
    juce::String wave2Percent;
    bool waveform = false;
    bool dragStarted{false};
    customLookAndFeel customLook;


};
class customSlider : public juce::Component, public juce::Slider::Listener, juce::Timer
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
        slider.setLookAndFeel(&customLook);

        setPrefix();
    }
    customSlider::~customSlider()
    {
        slider.setLookAndFeel(nullptr);
    };
    void timerCallback() override
    {
        label.setText(labelText,juce::dontSendNotification);
        stopTimer();
    }
    void paint(juce::Graphics& g) override
    {
    }

    void resized() override
    {
        label.setText(labelText,juce::dontSendNotification);
        juce::Rectangle<int> area = getLocalBounds();
        juce::Rectangle<int> labelArea = area.removeFromBottom(14);
        label.setBounds(labelArea.withSizeKeepingCentre(area.getWidth()*2, 14));
        slider.setBounds(area);
    }

    void sliderValueChanged(juce::Slider*) override
    {
        if(integer)
            sliderValue= juce::String((int)slider.getValue())+suffix;
        if(!integer)
        {
            auto maxValue = slider.getMaximum();
            float sliderV=0;
            if(maxValue<=1.f)
                sliderV = slider.getValue()*100.f;
            else
                sliderV = slider.getValue();
            sliderValue= juce::String(roundTwoDec(sliderV))+suffix;
        }


        label.setText(sliderValue,juce::dontSendNotification);
        if(!dragStarted)
            startTimer(50);
    }

    void sliderDragStarted(juce::Slider*slider) override
    {
        label.setText(sliderValue,juce::dontSendNotification);
        dragStarted = true;
    }


    void sliderDragEnded(juce::Slider*slider) override
    {
        label.setText(labelText,juce::dontSendNotification);
        dragStarted = false;
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
    bool dragStarted{false};
    juce::String suffix;
    customLookAndFeel customLook;

};
