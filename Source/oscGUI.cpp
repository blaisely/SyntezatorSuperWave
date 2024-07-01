/*
  ==============================================================================

    oscGUI.cpp
    Created: 13 Apr 2024 2:50:39pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>
#include "oscGUI.h"
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
oscGUI::oscGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    oscMenu_osc1.addItem("Sine", 1);
    oscMenu_osc1.addItem("Saw", 2);
    oscMenu_osc1.addItem("Square", 3);
    oscMenu_osc1.addItem("Triangle", 4);
    oscMenu_osc1.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&oscMenu_osc1);
    oscMenu_osc1.addListener(this);

    osc1Selection.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.state, "oscType_osc1", oscMenu_osc1));

    oscMenu_osc1.setJustificationType(juce::Justification::centred);

    oscMenu_osc2.addItem("Sine", 1);
    oscMenu_osc2.addItem("Saw", 2);
    oscMenu_osc2.addItem("Square", 3);
    oscMenu_osc2.addItem("Triangle", 4);
    addAndMakeVisible(&oscMenu_osc2);
    oscMenu_osc2.addListener(this);

    osc2Selection.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.state, "oscType_osc2", oscMenu_osc2));

    oscMenu_osc2.setJustificationType(juce::Justification::centred);


    Gain_osc1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    Gain_osc1.setRange(0.0f, 1.0f, 0.1f);
    Gain_osc1.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 25, 10);
    Gain_osc1.setValue(-20.0f);
    Gain_osc1Attach.reset(new SliderAttachment(audioProcessor.state, "gain_osc1", Gain_osc1));
   // Gain_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "fmfreq", fmFreq);
    //filterCutOffAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "filterCutoff", cutOffSlider));
    
    addAndMakeVisible(Gain_osc1);
    Gain_osc1.addListener(this);
    Gain_osc1_label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    Gain_osc1_label.setFont(15.0f);
    Gain_osc1_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(Gain_osc1_label);


    Gain_osc2.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    Gain_osc2.setRange(0.0f, 1.0f, 0.1f);
    Gain_osc2.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 25, 10);
    Gain_osc2.setValue(-60.0f);
    Gain_osc2Attach.reset(new SliderAttachment(audioProcessor.state, "gain_osc2", Gain_osc2));
   
    addAndMakeVisible(Gain_osc2);
    Gain_osc2.addListener(this);
    Gain_osc2_label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    Gain_osc2_label.setFont(15.0f);
    Gain_osc2_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(Gain_osc2_label);


   //detune

    detune.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    detune.setRange(0.0f, 1.0f, 0.01f);
    detune.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 25, 10);
    detune.setValue(0.0f);
    detuneAttach.reset(new SliderAttachment(audioProcessor.state, "detuneSuper", detune));
    // Gain_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "fmfreq", fmFreq);
     //filterCutOffAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "filterCutoff", cutOffSlider));

    addAndMakeVisible(detune);
    detune.addListener(this);
    detuneLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    detuneLabel.setFont(15.0f);
    detuneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(detuneLabel);

    //detune

    volume.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    volume.setRange(0.0f, 1.0f, 0.01f);
    volume.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 25, 10);
    volume.setValue(0.0f);
    volumeAttach.reset(new SliderAttachment(audioProcessor.state, "volumeSuper", volume));
    // Gain_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "fmfreq", fmFreq);
     //filterCutOffAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, "filterCutoff", cutOffSlider));

    addAndMakeVisible(volume);
    volume.addListener(this);
    volumeLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    volumeLabel.setFont(15.0f);
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel);

  /*  setSliderWithLabel(fmFreq, Gain_osc1_label, audioProcessor.state, fmFreqId, Gain_osc1Attach);
    setSliderWithLabel(fmDepth, Gain_osc2_label, audioProcessor.state, fmDepthId, Gain_osc2Attach);*/

    //detune
//-------------------------
    makeSlider(octave_osc1,  "octave_osc1", octave_osc1_label);
    octave_osc1Attach.reset(new SliderAttachment(audioProcessor.state, "octave_osc1", octave_osc1));
    makeSlider(octave_osc2, "octave_osc2", octave_osc2_label);
    octave_osc2Attach.reset(new SliderAttachment(audioProcessor.state, "octave_osc2", octave_osc2));
    makeSlider(coarse_osc1, "octave_osc1", octave_osc1_label);
    coarse_osc1Attach.reset(new SliderAttachment(audioProcessor.state, "coarse_osc1", coarse_osc1));
    makeSlider(coarse_osc2, "coarse_osc2", coarse_osc2_label);
    coarse_osc2Attach.reset(new SliderAttachment(audioProcessor.state, "coarse_osc2", coarse_osc2));

    
   

    setSize(400, 200);

}

oscGUI::~oscGUI()
{
}

void oscGUI::paint (juce::Graphics& g)
{

    g.fillAll(juce::Colours::black);   // clear the background

    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

}

void oscGUI::resized()
{
    
    juce::Rectangle<int> bottomArea = getLocalBounds().removeFromBottom(25);
    juce::Rectangle<int> topArea = getLocalBounds().removeFromTop(75);
    juce::Rectangle<int> middleArea(0, 75.0f, 400.0f, 100.0f);
 
    
    oscMenu_osc1.setBounds(topArea.removeFromLeft(60));
    octave_osc1.setBounds(topArea.removeFromLeft(60));
    coarse_osc1.setBounds(topArea.removeFromLeft(60));
    
    coarse_osc2.setBounds(topArea.removeFromRight(60));
    octave_osc2.setBounds(topArea.removeFromRight(60));
    oscMenu_osc2.setBounds(topArea.removeFromRight(60));
    Gain_osc1.setBounds(middleArea.removeFromLeft(100));
    detune.setBounds(middleArea.removeFromLeft(100));
    volume.setBounds(middleArea.removeFromLeft(100));
        Gain_osc2.setBounds(middleArea.removeFromLeft(100));
        Gain_osc1_label.setBounds(bottomArea.removeFromLeft(100));
        detuneLabel.setBounds(bottomArea.removeFromLeft(100));
            volumeLabel.setBounds(bottomArea.removeFromLeft(100));
        Gain_osc2_label.setBounds(bottomArea.removeFromRight(100));


}
void oscGUI::comboBoxChanged(juce::ComboBox* box) {

}

void oscGUI::sliderValueChanged(juce::Slider* slider)
{
}



void oscGUI::makeSlider(juce::Slider& slider, juce::String ID, juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
   
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    //attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.state, ID, slider));
    //attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, ID, slider);

    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

}