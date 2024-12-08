/*
  ==============================================================================

    oscGUI.cpp
    Created: 13 Apr 2024 2:50:39pm
    Author:  blaze

  ==============================================================================
*/



#include <memory>
#include "oscGUI.h"
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
oscGUI::oscGUI(SuperWaveSynthAudioProcessor& p) : audioProcessor(p), Gain_osc1("GAIN",2,false,true),Gain_osc2("GAIN",2,false,true),
detune("DETUNE",2,false),detuneMix("MIX",2,false),octave_osc1("OCTAVE",0,true),
octave_osc2("OCTAVE",0,true),coarse_osc1("FINE",3,true),coarse_osc2("FINE",3,true),
semiDetune_osc1("COARSE",0,true),semiDetune_osc2("COARSE",0,true),panOsc1("PAN",2,false,true),
panOsc2("PAN",2,false,true),waveTypeOSC1("WAVE",4,false),waveTypeOSC2("WAVE",4,false),
PWOSC1("PW",2,true),PWOSC2("PW",2,true)
{
    addAndMakeVisible(preset);
    preset.addItem("1",1);
    preset.setJustificationType(juce::Justification::centred);
    aliasingAttachment = std::make_unique<ButtonAttachment>(audioProcessor.state,"aliasing",aliasingON);
    aliasingON.setToggleable(true);
    aliasingON.setClickingTogglesState(true);
    aliasingON.setButtonText("Aliasing OFF");
    aliasingON.addListener(this);
    addAndMakeVisible(aliasingON);
    addAndMakeVisible(Gain_osc1);
    Gain_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "gain_osc1", Gain_osc1.slider);
    addAndMakeVisible(Gain_osc2);
    Gain_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "gain_osc2", Gain_osc2.slider);
    addAndMakeVisible(detune);
    detuneAttach = std::make_unique<SliderAttachment>(audioProcessor.state, "detuneSuper", detune.slider);
    addAndMakeVisible(detuneMix);
    volumeAttach = std::make_unique<SliderAttachment>(audioProcessor.state, "volumeSuper", detuneMix.slider);
    addAndMakeVisible(octave_osc1);
    octave_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "octave_osc1", octave_osc1.slider);
    addAndMakeVisible(octave_osc2);
    octave_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "octave_osc2", octave_osc2.slider);
    addAndMakeVisible(coarse_osc1);
    coarse_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "detune_osc1", coarse_osc1.slider);
    addAndMakeVisible(coarse_osc2);
    coarse_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "detune_osc2", coarse_osc2.slider);
    addAndMakeVisible(semiDetune_osc1);
    semiDetune_osc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "coarse_osc1",semiDetune_osc1.slider);
    addAndMakeVisible(semiDetune_osc2);
    semiDetune_osc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state, "coarse_osc2",semiDetune_osc2.slider);
    addAndMakeVisible(panOsc1);
    panOsc1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"panOsc1",panOsc1.slider);
    addAndMakeVisible(panOsc2);
    panOsc2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"panOsc2",panOsc2.slider);
    addAndMakeVisible(waveTypeOSC1);
    waveTypeOSC1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"oscType_osc1",waveTypeOSC1.slider);
    addAndMakeVisible(waveTypeOSC2);
    waveTypeOSC2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"oscType_osc2",waveTypeOSC2.slider);
    addAndMakeVisible(PWOSC1);
    PWOSC1Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"pulseWidthOsc1",PWOSC1.slider);
    addAndMakeVisible(PWOSC2);
    PWOSC2Attach = std::make_unique<SliderAttachment>(audioProcessor.state,"pulseWidthOsc2",PWOSC2.slider);

    OSC1.setJustificationType(juce::Justification::centred);
    OSC2.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(OSC1);
    addAndMakeVisible(OSC2);
    setSize(530, 295);

    OSC1.setLookAndFeel(&customLook);
    OSC2.setLookAndFeel(&customLook);
    aliasingON.setLookAndFeel(&noToggleLook);
}

oscGUI::~oscGUI()
{
    OSC1.setLookAndFeel(nullptr);
    OSC2.setLookAndFeel(nullptr);
    aliasingON.setLookAndFeel(nullptr);
}

void oscGUI::buttonClicked(juce::Button*)
{
}

void oscGUI::buttonStateChanged(juce::Button* button)
{
    if(button==&aliasingON)
    {
        if(aliasingON.getToggleState())
        {
            aliasingON.setButtonText("Aliasing ON");
        }
        if(!aliasingON.getToggleState())
        {
            aliasingON.setButtonText("Aliasing OFF");
        }
    }
}

void oscGUI::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(getLocalBounds().toFloat(),8);
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
    constexpr int knobSize = 80;
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
    juce::Rectangle<int> bottomLabelArea = bottomArea.removeFromTop(30).expanded(5);

    juce::FlexBox topLabel;
    topLabel.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(topLabel,OSC1,oscLabelWidth,oscLabelHeight,knobsMargin);
    addItemToFlexBox(topLabel,Gain_osc1,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(topLabel,panOsc1,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(topLabel,aliasingON,sliderWidth/2,sliderHeight,knobsMargin);
    addItemToFlexBox(topLabel,aliasingON,sliderWidth/2,20,5);
    topLabel.performLayout(topLabelArea);

    juce::FlexBox topSection;
    topSection.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox osc1Knobs;
    osc1Knobs.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(osc1Knobs,waveTypeOSC1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,PWOSC1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,detune,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,detuneMix,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,octave_osc1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,semiDetune_osc1,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc1Knobs,coarse_osc1,knobSize,knobSize,knobsMargin);


    topSection.items.add(juce::FlexItem(osc1Knobs).withFlex(1).withHeight(knobSize));
    topSection.performLayout(topArea);

    OSC2.setBounds(bottomLabelArea);

    juce::FlexBox bottomSection;
    bottomSection.flexDirection = juce::FlexBox::Direction::column;
    juce::FlexBox osc2Knobs;
    osc2Knobs.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(osc2Knobs,waveTypeOSC2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,PWOSC2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,octave_osc2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,semiDetune_osc2,knobSize,knobSize,knobsMargin);
    addItemToFlexBox(osc2Knobs,coarse_osc2,knobSize,knobSize,knobsMargin);


    bottomSection.items.add(juce::FlexItem(osc2Knobs).withFlex(1).withHeight(knobSize));
    bottomSection.performLayout(bottomArea);

    juce::FlexBox bottomLabel;
    bottomLabel.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(bottomLabel,OSC2,oscLabelWidth,oscLabelHeight,knobsMargin);
    addItemToFlexBox(bottomLabel,Gain_osc2,sliderWidth,sliderHeight,knobsMargin);
    addItemToFlexBox(bottomLabel,panOsc2,sliderWidth,sliderHeight,knobsMargin);
    bottomLabel.performLayout(bottomLabelArea);
}

void oscGUI::comboBoxChanged(juce::ComboBox* box) {

}

void oscGUI::sliderValueChanged(juce::Slider* slider)
{
}



