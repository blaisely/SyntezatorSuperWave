/*
  ==============================================================================

    Envelope.cpp
    Created: 14 Apr 2024 7:02:37pm
    Author:  blaze

  ==============================================================================
*/

#include <JuceHeader.h>

#include <memory>
#include "Envelope.h"

//==============================================================================
Envelope::Envelope(SimpleSynthAudioProcessor& p) : audioProcessor(p)
{
   makeSlider(attackAmp,attackAmpLabel);
   makeSlider(decayAmp,decayAmpLabel);
   makeSlider(sustainAmp,sustainAmpLabel);
   makeSlider(releaseAmp,releaseAmpLabel);
    makeSlider(attackMod,attackModLabel);
    makeSlider(decayMod,decayModLabel);
    makeSlider(sustainMod,sustainModLabel);
    makeSlider(releaseMod,releaseModLabel);
    makeSlider(modAmount,modAmountLabel);
    makeKnob(lfoDepth,lfoDepthLabel);
    makeKnob(lfoFreq,lfoFreqLabel);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "attack", attackAmp);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "decay", decayAmp);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "sustain", sustainAmp);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "release", releaseAmp);

    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"attackOsc2",attackMod);
    decayModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"decayOsc2",decayMod);
    sustainModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"sustainOsc2",sustainMod);
    releaseModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"releaseOsc2",releaseMod);
    modAmountAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"filterEnvelope",modAmount);
    loopEnvelopeAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"loopEnvelope",loopEnvelope);
    lfoDepthAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"lfodepth",lfoDepth);
    lfoFreqAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"lfofreq",lfoFreq);
    lfoUnipolarAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"lfo1Unipolar",lfoUnipolar);
    lfoResetAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"lfoReset",lfoReset);
    lfoTypeAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"lfoType",lfoType);

    addAndMakeVisible(modEnvType);
    modEnvType.setToggleable(true);
    modEnvType.setButtonText("Env 1");
    modEnvType.addListener(this);
    loopEnvelope.setButtonText("Loop");
    loopEnvelope.setToggleable(true);
    loopEnvelope.addListener(this);
    addAndMakeVisible(loopEnvelope);
    juce::StringArray lfoOptions{"Sine","Square","Saw","Sample&Hold"};
    lfoType.addItemList(lfoOptions,1);
    lfoType.setSelectedId(1);
    addAndMakeVisible(lfoType);
    lfoUnipolar.setButtonText("Unipolar");
    lfoUnipolar.setToggleable(true);
    lfoReset.setButtonText("Reset");
    lfoReset.setToggleable(true);
    addAndMakeVisible(lfoReset);
    addAndMakeVisible(lfoUnipolar);
    juce::StringArray lfoNumbers{"LFO1","LFO2","LFO3"};
    lfoNumber.addItemList(lfoNumbers,1);
    lfoNumber.setSelectedId(1);
    lfoNumber.addListener(this);
    lfoType.addListener(this);
    lfoUnipolar.addListener(this);
    lfoReset.addListener(this);
    addAndMakeVisible(lfoNumber);

    setSize(530, 150);
}

Envelope::~Envelope()
= default;

void Envelope::paint (juce::Graphics& g)
{
    juce::Rectangle<int> area = getLocalBounds();
    g.setColour(juce::Colour(0xff312F2F));
    g.fillRoundedRectangle(area.toFloat(),6);
   
}

void Envelope::resized()
{
    constexpr int sliderWidth = 25;
    constexpr int sliderHeight = 115;
    constexpr int margin  = 1;
    constexpr int labelWidth = 25;
    constexpr int labelHeight = 10;
    constexpr int buttonWidth = 40;
    constexpr int buttonHeight = 20;
    constexpr int amountSliderWidth = 20;
    constexpr int amountSliderHeight = 80;
    constexpr int lfoKnobSize =120;
    constexpr int lfoLabelWidth=120;
    constexpr int comboBoxWidth = 100;
    constexpr int comboBoxHeight = 20;
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> ampArea = area.removeFromLeft(122).reduced(5);
    juce::Rectangle<int> ampLabelArea = ampArea.removeFromBottom(10);
    juce::Rectangle<int> modEnvelope = area.removeFromLeft(122).reduced(5);
    juce::Rectangle<int> modLabelArea = modEnvelope.removeFromBottom(10);
    juce::Rectangle<int> envelopeButtonsArea = area.removeFromLeft(90).reduced(5);
    juce::Rectangle<int> envelopeButtonsLabelArea = envelopeButtonsArea.removeFromBottom(10).removeFromRight(60);
    juce::Rectangle<int> amountSliderArea = envelopeButtonsArea.removeFromRight(40);
    juce::Rectangle<int> lfoArea = area.removeFromLeft(170).reduced(5);
    juce::Rectangle<int> lfoKnobsArea = lfoArea.removeFromLeft(100);
    juce::Rectangle<int> lfoKnobsLabel = lfoKnobsArea.removeFromBottom(10);


    juce::FlexBox amp;
    amp.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(amp,attackAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,decayAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,sustainAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,releaseAmp,sliderWidth,sliderHeight,margin);
    amp.performLayout(ampArea);

    juce::FlexBox ampLabels;
    ampLabels.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(ampLabels,attackAmpLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(ampLabels,decayAmpLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(ampLabels,sustainAmpLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(ampLabels,releaseAmpLabel,labelWidth,labelHeight,margin);
    ampLabels.performLayout(ampLabelArea);

    juce::FlexBox mod;
    mod.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(mod,attackMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,decayMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,sustainMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,releaseMod,sliderWidth,sliderHeight,margin);
    mod.performLayout(modEnvelope);

    juce::FlexBox modLabels;
    modLabels.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(modLabels,attackModLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(modLabels,decayModLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(modLabels,sustainModLabel,labelWidth,labelHeight,margin);
    addItemToFlexBox(modLabels,releaseModLabel,labelWidth,labelHeight,margin);
    modLabels.performLayout(modLabelArea);

    juce::FlexBox modControls;
    modControls.flexDirection = juce::FlexBox::Direction::column;
    modControls.alignContent = juce::FlexBox::AlignContent::spaceAround;
    addItemToFlexBoxWithAlign(modControls,modEnvType,buttonWidth,buttonHeight,1);
    addItemToFlexBoxWithAlign(modControls,loopEnvelope,buttonWidth,buttonHeight,1);
    modControls.performLayout(envelopeButtonsArea);
    modAmount.setBounds(amountSliderArea);
    modAmountLabel.setBounds(envelopeButtonsLabelArea);

    juce::FlexBox lfoKnobs;
    lfoKnobs.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(lfoKnobs,lfoDepth, lfoKnobSize,lfoKnobSize,margin);
    addItemToFlexBox(lfoKnobs,lfoDepthLabel,lfoLabelWidth,labelHeight,margin);
    addItemToFlexBox(lfoKnobs,lfoFreq,lfoKnobSize,lfoKnobSize,margin);
    lfoKnobs.performLayout(lfoKnobsArea);
    lfoFreqLabel.setBounds(lfoKnobsLabel);

    juce::FlexBox lfoControls;
    lfoControls.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(lfoControls,lfoNumber,comboBoxWidth,comboBoxHeight,margin);
    addItemToFlexBox(lfoControls,lfoType,comboBoxWidth,comboBoxHeight,margin);
    addItemToFlexBox(lfoControls,lfoReset,buttonWidth,buttonHeight,margin);
    addItemToFlexBox(lfoControls,lfoUnipolar,buttonWidth,buttonHeight,margin);
    lfoControls.performLayout(lfoArea);

}

void Envelope::sliderValueChanged(juce::Slider* slider)
{
}

void Envelope::buttonStateChanged(juce::Button* button)
{

}

void Envelope::buttonClicked(juce::Button* button)
{
    bool state = true;
    bool state2= true;
    bool state3= true;
    if(button==&modEnvType)
    {
        if(!button->getToggleState())
        {
            button->setButtonText("Env 2");
            changeEnvelopes(2);
        }

        else if(button->getToggleState())
        {
            button->setButtonText("Env 1");
            changeEnvelopes(1);
        }

        state= button->getToggleState();
        button->setToggleState(!state,juce::dontSendNotification);
    }
    if(button==&loopEnvelope)
    {
        state3 = button->getToggleState();
        button->setToggleState(!state3,juce::dontSendNotification);
    }
    if(button==&lfoReset)
    {
        state3 = button->getToggleState();
        button->setToggleState(!state3,juce::dontSendNotification);
    }
    if(button==&lfoUnipolar)
    {
        state3 = button->getToggleState();
        button->setToggleState(!state3,juce::dontSendNotification);
    }
}

void Envelope::makeKnob(juce::Slider& slider,juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(12.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void Envelope::makeSlider(juce::Slider& slider, juce::Label& label)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 5.0f, 10.0f);
    addAndMakeVisible(&slider);
    slider.addListener(this);
    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(12.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void Envelope::setUpLFOAttachments(juce::StringArray& id)
{
    lfoDepthAttach = std::make_unique<SliderAttachment>(audioProcessor.state,id[0],lfoDepth);
    lfoFreqAttach = std::make_unique<SliderAttachment>(audioProcessor.state,id[1],lfoFreq);
    lfoUnipolarAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,id[2],lfoUnipolar);
    lfoResetAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,id[3],lfoReset);
    lfoTypeAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,id[4],lfoType);
}

void Envelope::setUpLFOKnobs(juce::StringArray& id)
{
    lfoDepth.setValue(audioProcessor.state.getRawParameterValue(id[0])->load());
    lfoFreq.setValue(audioProcessor.state.getRawParameterValue(id[1])->load());
    lfoUnipolar.setToggleState(audioProcessor.state.getRawParameterValue(id[2])->load(),juce::dontSendNotification);
    lfoReset.setToggleState(audioProcessor.state.getRawParameterValue(id[3])->load(),juce::dontSendNotification);
    lfoType.setSelectedId(audioProcessor.state.getRawParameterValue(id[4])->load()+1);

}

void Envelope::setUpModAttachments(juce::StringArray& id)
{
    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[0],attackMod);
    decayModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[1],decayMod);
    sustainModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[2],sustainMod);
    releaseModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[3],releaseMod);
    modAmountAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[4],modAmount);
    loopEnvelopeAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,id[5],loopEnvelope);
}

void Envelope::setUpModSliders(juce::StringArray& id)
{
    attackMod.setValue(audioProcessor.state.getRawParameterValue(id[0])->load());
    decayMod.setValue(audioProcessor.state.getRawParameterValue(id[1])->load());
    sustainMod.setValue(audioProcessor.state.getRawParameterValue(id[2])->load());
    releaseMod.setValue(audioProcessor.state.getRawParameterValue(id[3])->load());
    modAmount.setValue(audioProcessor.state.getRawParameterValue(id[4])->load());
    loopEnvelope.setToggleState(audioProcessor.state.getRawParameterValue(id[5])->load(),juce::dontSendNotification);
}

void Envelope::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if(comboBoxThatHasChanged==&lfoNumber)
    {
        if(lfoNumber.getSelectedId()==1)
        {
        setUpLFOKnobs(LFO1IDs);
        setUpLFOAttachments(LFO1IDs);
        }
        if(lfoNumber.getSelectedId()==2)
        {
            setUpLFOKnobs(LFO2IDs);
            setUpLFOAttachments(LFO2IDs);
        }
        if(lfoNumber.getSelectedId()==3)
        {
            setUpLFOKnobs(LFO3IDs);
            setUpLFOAttachments(LFO3IDs);
        }
    }
}

void Envelope::changeEnvelopes(const int envelope)
{
    if(envelope==1)
    {
        setUpModSliders(envelope2IDs);
        setUpModAttachments(envelope2IDs);
    }
    else if(envelope==2)
    {
        setUpModSliders(envelope3IDs);
        setUpModAttachments(envelope3IDs);
    }
}
