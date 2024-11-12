/*
==============================================================================

    customLookAndFeel.h
    Created: 10 Jul 2024 11:07:30am
    Author:  blaze

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class customLookAndFeel : public juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
};