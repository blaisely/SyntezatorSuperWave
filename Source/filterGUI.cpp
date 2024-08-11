/*
  ==============================================================================

    filterGUI.cpp
    Created: 13 Apr 2024 3:03:55pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>
#include "filterGUI.h"



typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
filterGUI::filterGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    cutOffSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    cutOffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
    filterCutOffAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "filterCutoff", cutOffSlider));
    

    addAndMakeVisible(&cutOffSlider);
    cutOffSlider.addListener(this);
    cutOffSlider.setSkewFactorFromMidPoint(1000.0f);

    filterResonance.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    filterResonance.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
    filterResonanceAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "filterRes", filterResonance));

    addAndMakeVisible(&filterResonance);
    filterResonance.addListener(this);

    filterMenu.addItem("LowPass", 1);
    filterMenu.addItem("HighPass", 2);
    filterMenu.addItem("BandPass", 3);
    addAndMakeVisible(filterMenu);
    filterMenu.addListener(this);
    filterSelection.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.state, "filterType", filterMenu));
    filterMenu.setJustificationType(juce::Justification::centred);

    makeSlider(lfoFreq, lfoFreqAttach, ID_lfoFreq, lfoFreqLabel, 0.0f, 100.0f, 0.0f);
    makeSlider(lfoDepth, lfoDepthAttach, ID_lfoDepth, lfoDepthLabel, 0.0f, 1000.0f, 0.0f);

    filterOn.setButtonText("Digital");
    buttonAttach.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.state, "filterbutton", filterOn));
    addAndMakeVisible(filterOn);
    filterOn.addListener(this);

    
    setSize(200, 400);

}

filterGUI::~filterGUI()
{
}

void filterGUI::paint (juce::Graphics& g)
{
    
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);  
   

}

void filterGUI::resized()
{
   
    juce::Rectangle<int> area = getLocalBounds();
    const int sliderWidth = 100;
    const int sliderHeight = 100;
    juce::Rectangle<int> topSection = getLocalBounds().removeFromTop(150);
    juce::Rectangle<int> bottomSection = getLocalBounds().removeFromBottom(150);
    juce::Rectangle<int> middleSection = { 0,150,150,50 };
    juce::Rectangle<int> middleSection3 = { 150,150,50,50 };
    juce::Rectangle<int> middleSection_2 = { 0,200,200,50 };
    cutOffSlider.setBounds(topSection.removeFromLeft(100));
    filterResonance.setBounds(topSection.removeFromLeft(100));
    filterMenu.setBounds(middleSection);
    filterOn.setBounds(middleSection3.reduced(5));
    lfoDepth.setBounds(bottomSection.removeFromLeft(100));
    lfoFreq.setBounds(bottomSection);
    lfoDepthLabel.setBounds(middleSection_2.removeFromLeft(100));
    lfoFreqLabel.setBounds(middleSection_2);

}

void filterGUI::comboBoxChanged(juce::ComboBox* box)
{
}

void filterGUI::sliderValueChanged(juce::Slider* slider)
{
}

void filterGUI::buttonClicked(juce::Button* button)
{
    
    if (button == &filterOn)
    {
        bool currentValue = audioProcessor.state.getParameterAsValue("filterbutton").getValue();
        if (currentValue)
        {
            filterOn.setButtonText("DIGITAL");
        }
        else
        {
            filterOn.setButtonText("SVF");
        }
        audioProcessor.state.getParameterAsValue("filterbutton").setValue(!currentValue);
    }
}

void filterGUI::makeSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::String ID, juce::Label& label, float initValue, float maxValue,float minValue)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setRange(minValue, maxValue);
    slider.setValue(initValue);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, ID, slider);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

}
