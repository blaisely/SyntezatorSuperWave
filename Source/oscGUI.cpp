/*
  ==============================================================================

    oscGUI.cpp
    Created: 13 Apr 2024 2:50:39pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>

#include <memory>
#include "oscGUI.h"

#include "juce_core/unit_tests/juce_UnitTestCategories.h"
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
oscGUI::oscGUI(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
    addAndMakeVisible(preset);
    preset.addItem("1",1);
    preset.setJustificationType(juce::Justification::centred);

    makeSlider(Gain_osc1,Gain_osc1_label);
    Gain_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "gain_osc1", Gain_osc1);
    makeSlider(Gain_osc2,Gain_osc2_label);
    Gain_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "gain_osc2", Gain_osc2);
    makeKnob(detune,"detuneSuper",detuneLabel);
    detuneAttach = std::make_unique<SliderAttachment>(audioProcessor.state, "detuneSuper", detune);
    makeKnob(detuneMix,"volumeSuper",detuneMixLabel);
    volumeAttach = std::make_unique<SliderAttachment>(audioProcessor.state, "volumeSuper", detuneMix);
    makeKnob(octave_osc1,  "octave_osc1", octave_osc1_label);
    octave_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "octave_osc1", octave_osc1);
    makeKnob(octave_osc2, "octave_osc2", octave_osc2_label);
    octave_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "octave_osc2", octave_osc2);
    makeKnob(coarse_osc1, "octave_osc1", coarse_osc1_label);
    coarse_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "coarse_osc1", coarse_osc1);
    makeKnob(coarse_osc2, "coarse_osc2", coarse_osc2_label);
    coarse_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "coarse_osc2", coarse_osc2);
    makeKnob(semiDetune_osc1, "detune_osc1",semiDetune_osc1_label);
    semiDetune_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "detune_osc1",semiDetune_osc1);
    makeKnob(semiDetune_osc2, "detune_osc2",semiDetune_osc2_label);
    semiDetune_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "detune_osc2",semiDetune_osc2);
    makeSlider(panOsc1,panOsc1Label);
    panOsc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"panOsc1",panOsc1);
    makeSlider(panOsc2,panOsc2Label);
    panOsc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"panOsc2",panOsc2);
    makeKnob(waveTypeOSC1,"oscType_osc1",waveTypeLabel1);
    waveTypeOSC1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"oscType_osc1",waveTypeOSC1);
    makeKnob(waveTypeOSC2,"oscType_osc2",waveTypeLabel2);
    waveTypeOSC2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"oscType_osc2",waveTypeOSC2);
    makeKnob(PWOSC1,"pulseWidthOsc1",PWMLabel1);
    PWOSC1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"pulseWidthOsc1",PWOSC1);
    makeKnob(PWOSC2,"pulseWidthOsc2",PWMLabel2);
    PWOSC2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"pulseWidthOsc2",PWOSC2);

    //TODO incorporate into a separate Slider class
    /*PWOSC1.onDragStart = [this]()
    {
        PWOSC1.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 40.0f, 20.0f);

    };
    PWOSC1.onDragEnd = [this]()
    {
        PWOSC1.setTextBoxStyle(juce::Slider::NoTextBox, true, 20.0f, 10.0f);
    };*/
    addAndMakeVisible(OSC1);
    addAndMakeVisible(OSC2);
    Gain_osc1_label.setFont(10.f);
    Gain_osc2_label.setFont(10.f);
    panOsc1Label.setFont(10.f);
    panOsc2Label.setFont(10.f);
    setSize(530, 295);
}

oscGUI::~oscGUI()
{
}

void oscGUI::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(getLocalBounds().toFloat(),6);
    juce::Rectangle<int> totalArea = getLocalBounds();

    juce::Rectangle<int> topArea = totalArea.removeFromTop(137).reduced(5,2);
    juce::Rectangle<int> bottomArea = totalArea.removeFromBottom(137).reduced(5,2);
    juce::Rectangle<int> topLabelArea = topArea.removeFromTop(30).expanded(5);
    juce::Rectangle<int> topKnobNameArea = topArea.removeFromBottom(5);
    juce::Rectangle<int> bottomLabelArea = bottomArea.removeFromTop(30).expanded(5);
    juce::Rectangle<int> bottomKnobNameArea = bottomArea.removeFromBottom(5);
    g.setColour(juce::Colour(0x54DBD9F4));
    g.fillRoundedRectangle(topArea.toFloat(),6);
    g.fillRoundedRectangle(bottomArea.toFloat(),6);
}

void oscGUI::resized()
{
    constexpr int knobsMargin = 1;
    constexpr int knobSize = 65;
    constexpr int oscLabelWidth = 50;
    constexpr int oscLabelHeight = 30;
    constexpr int sliderWidth = 125;
    constexpr int sliderHeight = 30;
    constexpr int knobLabelHeight = 10;
    constexpr int knobLabelWidth = 65;

    juce::Rectangle<int> totalArea = getLocalBounds();

    juce::Rectangle<int> topArea = totalArea.removeFromTop(137).reduced(5).reduced(10,0);
    juce::Rectangle<int> bottomArea = totalArea.removeFromBottom(137).reduced(5).reduced(10,0);

    juce::Rectangle<int> topLabelArea = topArea.removeFromTop(30).expanded(5);
    juce::Rectangle<int> topKnobNameArea = topArea.removeFromBottom(15);
    juce::Rectangle<int> bottomLabelArea = bottomArea.removeFromTop(30).expanded(5);
    juce::Rectangle<int> bottomKnobNameArea = bottomArea.removeFromBottom(15);

    juce::FlexBox topLabel;
    topLabel.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(topLabel,OSC1,oscLabelWidth,oscLabelHeight,knobsMargin);
    addItemToFlexBox(topLabel,Gain_osc1,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(topLabel,Gain_osc1_label,oscLabelWidth,oscLabelHeight,knobsMargin);
    addItemToFlexBox(topLabel,panOsc1,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(topLabel,panOsc1Label,oscLabelWidth,oscLabelHeight,knobsMargin);
    topLabel.performLayout(topLabelArea);

    juce::FlexBox osc1Knobs;
    osc1Knobs.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(osc1Knobs,waveTypeOSC1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,PWOSC1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,detune,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,detuneMix,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,octave_osc1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,coarse_osc1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,semiDetune_osc1,knobSize,knobSize,knobsMargin);
    osc1Knobs.performLayout(topArea);

    juce::FlexBox osc1Labels;
    osc1Labels.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(osc1Labels,waveTypeLabel1,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc1Labels,PWMLabel1,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc1Labels,detuneLabel,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc1Labels,detuneMixLabel,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc1Labels,octave_osc1_label,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc1Labels,coarse_osc1_label,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc1Labels,semiDetune_osc1_label,knobLabelWidth,knobLabelHeight,1);
    osc1Labels.performLayout(topKnobNameArea);

    OSC2.setBounds(bottomLabelArea);
    juce::FlexBox osc2Knobs;
    osc2Knobs.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(osc2Knobs,waveTypeOSC2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,PWOSC2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,octave_osc2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,coarse_osc2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,semiDetune_osc2,knobSize,knobSize,knobsMargin);
    osc2Knobs.performLayout(bottomArea);

    juce::FlexBox osc2Labels;
    osc2Labels.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(osc2Labels,waveTypeLabel2,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc2Labels,PWMLabel2,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc2Labels,octave_osc2_label,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc2Labels,coarse_osc2_label,knobLabelWidth,knobLabelHeight,1);
    addItemToFlexBox(osc2Labels,semiDetune_osc2_label,knobLabelWidth,knobLabelHeight,1);
    osc2Labels.performLayout(bottomKnobNameArea);

    juce::FlexBox bottomLabel;
    bottomLabel.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(bottomLabel,OSC2,oscLabelWidth,oscLabelHeight,knobsMargin);
    addItemToFlexBox(bottomLabel,Gain_osc2,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(bottomLabel,Gain_osc2_label,oscLabelWidth,oscLabelHeight,knobsMargin);
    addItemToFlexBox(bottomLabel,panOsc2,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(bottomLabel,panOsc2Label,oscLabelWidth,oscLabelHeight,knobsMargin);
    bottomLabel.performLayout(bottomLabelArea);



}

void oscGUI::comboBoxChanged(juce::ComboBox* box) {

}

void oscGUI::sliderValueChanged(juce::Slider* slider)
{
}

void oscGUI::makeKnob(juce::Slider& slider, juce::String ID, juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 20.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void oscGUI::makeSlider(juce::Slider& slider, juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

