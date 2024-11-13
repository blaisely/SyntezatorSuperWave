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
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
};
class filterEmuLookAndFeel : public juce::LookAndFeel_V4
{
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};
class filterLabel : public juce::LookAndFeel_V4
{
    void drawLabel(juce::Graphics&, juce::Label&) override;
};