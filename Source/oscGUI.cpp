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
    oscMenu_osc1.addItem("Sine", 1);
    oscMenu_osc1.addItem("Saw", 2);
    oscMenu_osc1.addItem("Square", 3);
    oscMenu_osc1.addItem("Triangle", 4);
    oscMenu_osc1.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&oscMenu_osc1);
    oscMenu_osc1.addListener(this);

    osc1Selection = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.state, "oscType_osc1", oscMenu_osc1);

    oscMenu_osc1.setJustificationType(juce::Justification::centred);

    oscMenu_osc2.addItem("Sine", 1);
    oscMenu_osc2.addItem("Saw", 2);
    oscMenu_osc2.addItem("Square", 3);
    oscMenu_osc2.addItem("Triangle", 4);
    addAndMakeVisible(&oscMenu_osc2);
    oscMenu_osc2.addListener(this);

    osc2Selection = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.state, "oscType_osc2", oscMenu_osc2);

    oscMenu_osc2.setJustificationType(juce::Justification::centred);


    Gain_osc1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    Gain_osc1.setRange(0.0f, 1.0f, 0.1f);
    Gain_osc1.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 30);
    Gain_osc1.setValue(-20.0f);
    Gain_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "gain_osc1", Gain_osc1);
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
    Gain_osc2.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 30);
    Gain_osc2.setValue(-60.0f);
    Gain_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "gain_osc2", Gain_osc2);
   
    addAndMakeVisible(Gain_osc2);
    Gain_osc2.addListener(this);
    Gain_osc2_label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    Gain_osc2_label.setFont(15.0f);
    Gain_osc2_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(Gain_osc2_label);


   //detune

    detune.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    detune.setRange(0.0f, 1.0f, 0.01f);
    detune.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
    detune.setValue(0.0f);
    detuneAttach = std::make_unique<SliderAttachment>(audioProcessor.state, "detuneSuper", detune);
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
    volume.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 30);
    volume.setValue(0.0f);
    volumeAttach = std::make_unique<SliderAttachment>(audioProcessor.state, "volumeSuper", volume);

    addAndMakeVisible(volume);
    volume.addListener(this);
    volumeLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    volumeLabel.setFont(15.0f);
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel);

    //detune
//-------------------------
    makeSlider(octave_osc1,  "octave_osc1", octave_osc1_label);
    octave_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "octave_osc1", octave_osc1);
    makeSlider(octave_osc2, "octave_osc2", octave_osc2_label);
    octave_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "octave_osc2", octave_osc2);
    makeSlider(coarse_osc1, "octave_osc1", octave_osc1_label);
    coarse_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "coarse_osc1", coarse_osc1);
    makeSlider(coarse_osc2, "coarse_osc2", coarse_osc2_label);
    coarse_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "coarse_osc2", coarse_osc2);
    makeSlider(panOsc1,"panOsc1",panOsc1Label);
    panOsc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"panOsc1",panOsc1);
    makeSlider(panOsc2,"panOsc2",panOsc2Label);
    panOsc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"panOsc2",panOsc2);
    setSize(550, 350);

}

oscGUI::~oscGUI()
{
}

void oscGUI::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(getLocalBounds().toFloat(),6);
}

void oscGUI::setUpLabels(juce::FlexBox& labelsTop,juce::Label& firstLabel,juce::Label& secondLabel)
{
    labelsTop.flexDirection = juce::FlexBox::Direction::row;
    labelsTop.alignItems = juce::FlexBox::AlignItems::center;
    labelsTop.items.add(juce::FlexItem(firstLabel).withWidth(125).withHeight(15).withMargin(5));
    labelsTop.items.add(juce::FlexItem(secondLabel).withWidth(125).withHeight(15).withMargin(5));
}

void oscGUI::setUpKnobs(juce::FlexBox& flex,juce::Slider& firstSlider,juce::Slider& secondSlider)
{

    flex.flexDirection = juce::FlexBox::Direction::row;
    flex.alignItems = juce::FlexBox::AlignItems::center;
    flex.items.add(juce::FlexItem(firstSlider).withWidth(125).withHeight(125).withMargin(5));
    flex.items.add(juce::FlexItem(secondSlider).withWidth(125).withHeight(125).withMargin(5));
}
void oscGUI::setUpFlex(juce::FlexBox& flex)
{
    flex.flexDirection = juce::FlexBox::Direction::row;
    flex.alignItems = juce::FlexBox::AlignItems::center;
}


void oscGUI::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> totalArea = area.removeFromRight(300).removeFromTop(300).reduced(10);
    juce::Rectangle<int> leftArea = area.removeFromLeft(200);

    juce::FlexBox mainFlexBox;
    mainFlexBox.flexDirection = juce::FlexBox::Direction::column;
    mainFlexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    mainFlexBox.alignItems = juce::FlexBox::AlignItems::stretch;

    juce::FlexBox waveformFlexBox;
    waveformFlexBox.flexDirection = juce::FlexBox::Direction::row;
    waveformFlexBox.items.add(juce::FlexItem(oscMenu_osc1).withMinWidth(125.0f).withFlex(1.0f).withMargin(5));
    waveformFlexBox.items.add(juce::FlexItem(oscMenu_osc2).withMinWidth(125.0f).withFlex(1.0f).withMargin(5));

    waveformFlexBox.items.add(juce::FlexItem().withWidth(10.0f));

    mainFlexBox.items.add(juce::FlexItem(waveformFlexBox).withHeight(50.0f).withMargin(juce::FlexItem::Margin(5)));


    juce::FlexBox osc1SliderBox;
    osc1SliderBox.flexDirection = juce::FlexBox::Direction::column;


    juce::FlexBox osc1Row1;
    osc1Row1.flexDirection = juce::FlexBox::Direction::row;

    juce::FlexBox gainOsc1Box;
    gainOsc1Box.flexDirection = juce::FlexBox::Direction::column;
    gainOsc1Box.items.add(juce::FlexItem(Gain_osc1).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    gainOsc1Box.items.add(juce::FlexItem(Gain_osc1_label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    juce::FlexBox panOsc1Box;
    panOsc1Box.flexDirection = juce::FlexBox::Direction::column;
    panOsc1Box.items.add(juce::FlexItem(panOsc1).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    panOsc1Box.items.add(juce::FlexItem(panOsc1Label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    osc1Row1.items.add(juce::FlexItem(gainOsc1Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));
    osc1Row1.items.add(juce::FlexItem(panOsc1Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));

    juce::FlexBox osc1Row2;
    osc1Row2.flexDirection = juce::FlexBox::Direction::row;

    juce::FlexBox octaveOsc1Box;
    octaveOsc1Box.flexDirection = juce::FlexBox::Direction::column;
    octaveOsc1Box.items.add(juce::FlexItem(octave_osc1).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    octaveOsc1Box.items.add(juce::FlexItem(octave_osc1_label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    juce::FlexBox coarseOsc1Box;
    coarseOsc1Box.flexDirection = juce::FlexBox::Direction::column;
    coarseOsc1Box.items.add(juce::FlexItem(coarse_osc1).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    coarseOsc1Box.items.add(juce::FlexItem(coarse_osc1_label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    osc1Row2.items.add(juce::FlexItem(octaveOsc1Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));
    osc1Row2.items.add(juce::FlexItem(coarseOsc1Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));

    osc1SliderBox.items.add(juce::FlexItem(osc1Row1).withFlex(1.0f));
    osc1SliderBox.items.add(juce::FlexItem(osc1Row2).withFlex(1.0f));


    juce::FlexBox osc2SliderBox;
    osc2SliderBox.flexDirection = juce::FlexBox::Direction::column;


    juce::FlexBox osc2Row1;
    osc2Row1.flexDirection = juce::FlexBox::Direction::row;

    juce::FlexBox gainOsc2Box;
    gainOsc2Box.flexDirection = juce::FlexBox::Direction::column;
    gainOsc2Box.items.add(juce::FlexItem(Gain_osc2).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    gainOsc2Box.items.add(juce::FlexItem(Gain_osc2_label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    juce::FlexBox panOsc2Box;
    panOsc2Box.flexDirection = juce::FlexBox::Direction::column;
    panOsc2Box.items.add(juce::FlexItem(panOsc2).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    panOsc2Box.items.add(juce::FlexItem(panOsc2Label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    osc2Row1.items.add(juce::FlexItem(gainOsc2Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));
    osc2Row1.items.add(juce::FlexItem(panOsc2Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));


    juce::FlexBox osc2Row2;
    osc2Row2.flexDirection = juce::FlexBox::Direction::row;

    juce::FlexBox octaveOsc2Box;
    octaveOsc2Box.flexDirection = juce::FlexBox::Direction::column;
    octaveOsc2Box.items.add(juce::FlexItem(octave_osc2).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    octaveOsc2Box.items.add(juce::FlexItem(octave_osc2_label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    juce::FlexBox coarseOsc2Box;
    coarseOsc2Box.flexDirection = juce::FlexBox::Direction::column;
    coarseOsc2Box.items.add(juce::FlexItem(coarse_osc2).withMinWidth(75.0f).withMinHeight(75.0f).withFlex(1.0f));
    coarseOsc2Box.items.add(juce::FlexItem(coarse_osc2_label).withMinHeight(20.0f).withFlex(0.1f)); // Label under slider

    osc2Row2.items.add(juce::FlexItem(octaveOsc2Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));
    osc2Row2.items.add(juce::FlexItem(coarseOsc2Box).withFlex(1.0f).withMargin(juce::FlexItem::Margin(5)));

    osc2SliderBox.items.add(juce::FlexItem(osc2Row1).withFlex(1.0f));
    osc2SliderBox.items.add(juce::FlexItem(osc2Row2).withFlex(1.0f));


    juce::FlexBox oscSliderFlexBox;
    oscSliderFlexBox.flexDirection = juce::FlexBox::Direction::row;

    oscSliderFlexBox.items.add(juce::FlexItem(osc1SliderBox).withFlex(1.0f).withMargin(juce::FlexItem::Margin(10)));
    oscSliderFlexBox.items.add(juce::FlexItem(osc2SliderBox).withFlex(1.0f).withMargin(juce::FlexItem::Margin(10)));

    mainFlexBox.items.add(juce::FlexItem(oscSliderFlexBox).withFlex(1.0f));


    mainFlexBox.performLayout(totalArea);
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

void oscGUI::setUpRightOsc(const int& width)
{
}

