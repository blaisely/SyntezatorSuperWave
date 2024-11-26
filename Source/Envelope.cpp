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
Envelope::Envelope(SuperWaveSynthAudioProcessor& p) : audioProcessor(p),attackAmp("A",2,false,false),
decayAmp("D",2,false,false),sustainAmp("S",2,false,false),releaseAmp("R",2,false,false),
attackMod("A",2,false,false),
decayMod("D",2,false,false),sustainMod("S",2,false,false),releaseMod("R",2,false,false),
lfoDepth("Depth",2,true),lfoFreq("Freq",1,false),modAmount("Amount",2,true,false)
{

    addAndMakeVisible(attackAmp);
    addAndMakeVisible(decayAmp);
    addAndMakeVisible(sustainAmp);
    addAndMakeVisible(releaseAmp);
    addAndMakeVisible(attackMod);
    addAndMakeVisible(decayMod);
    addAndMakeVisible(sustainMod);
    addAndMakeVisible(releaseMod);
    addAndMakeVisible(modAmount);
    addAndMakeVisible(lfoDepth);
    addAndMakeVisible(lfoFreq);


    juce::StringArray lfoOptions{"Sine","Square","Saw","Sample&Hold"};
    lfoType.addItemList(lfoOptions,1);
    lfoType.addListener(this);
    addAndMakeVisible(lfoType);

    modEnvType.setToggleable(true);
    modEnvType.setClickingTogglesState(true);
    modEnvType.setButtonText("Env 1");
    modEnvType.addListener(this);

    loopEnvelope.setButtonText("Loop");
    loopEnvelope.setClickingTogglesState(true);
    loopEnvelope.setToggleable(true);
    loopEnvelope.addListener(this);
    addAndMakeVisible(loopEnvelope);

    lfoUnipolar.setButtonText("Unipolar");
    lfoUnipolar.setToggleable(true);
    lfoUnipolar.setClickingTogglesState(true);
    lfoUnipolar.addListener(this);
    addAndMakeVisible(lfoUnipolar);

    lfoReset.setButtonText("Reset");
    lfoReset.setClickingTogglesState(true);
    lfoReset.setToggleable(true);
    lfoReset.addListener(this);
    addAndMakeVisible(lfoReset);

    juce::StringArray lfoNumbers{"LFO1","LFO2","LFO3"};
    lfoNumber.addItemList(lfoNumbers,1);
    lfoNumber.addListener(this);
    addAndMakeVisible(lfoNumber);

    addAndMakeVisible(sharedAmp);
    sharedAmp.setToggleable(true);
    sharedAmp.setButtonText("Env1 -> OSC2");
    sharedAmp.setClickingTogglesState(true);
    sharedAmp.addListener(this);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "attack", attackAmp.slider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "decay", decayAmp.slider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "sustain", sustainAmp.slider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, "release", releaseAmp.slider);

    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"attackOsc2",attackMod.slider);
    decayModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"decayOsc2",decayMod.slider);
    sustainModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"sustainOsc2",sustainMod.slider);
    releaseModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"releaseOsc2",releaseMod.slider);
    modAmountAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,"filterEnvelope",modAmount.slider);
    loopEnvelopeAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"loopEnvelope",loopEnvelope);
    lfoDepthAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"lfodepth",lfoDepth.slider);
    lfoFreqAttach = std::make_unique<SliderAttachment>(audioProcessor.state,"lfofreq",lfoFreq.slider);
    lfoUnipolarAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"lfo1Unipolar",lfoUnipolar);
    lfoResetAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"lfoReset",lfoReset);
    sharedAmpAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,"commonEnvelope",sharedAmp);
    lfoTypeAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"lfoType",lfoType);
    lfoNumberAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,"lfoNumber",lfoNumber);
    addAndMakeVisible(modEnvType);



    lfoReset.setLookAndFeel(&customLook);
    loopEnvelope.setLookAndFeel(&customLook);
    sharedAmp.setLookAndFeel(&customLook);
    modEnvType.setLookAndFeel(&noToggleLook);
    lfoUnipolar.setLookAndFeel(&customLook);
    lfoNumber.setLookAndFeel(&comboBoxLook);
    lfoType.setLookAndFeel(&comboBoxLook);
    setSize(550, 160);
}

Envelope::~Envelope()
{

    lfoReset.setLookAndFeel(nullptr);
    loopEnvelope.setLookAndFeel(nullptr);
    sharedAmp.setLookAndFeel(nullptr);
    modEnvType.setLookAndFeel(nullptr);
    lfoUnipolar.setLookAndFeel(nullptr);
    lfoNumber.setLookAndFeel(nullptr);
    lfoType.setLookAndFeel(nullptr);
}


void Envelope::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(getLocalBounds().toFloat(),8);
    juce::Rectangle<int> totalArea = getLocalBounds();

    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> ampArea = area.removeFromLeft(122).reduced(5);
    juce::Rectangle<int> modEnvelope = area.removeFromLeft(217).reduced(5);
    juce::Rectangle<int> lfoArea = area.removeFromLeft(150).reduced(5);

    g.setColour(juce::Colour(0x54DBD9F4));
    g.fillRoundedRectangle(ampArea.toFloat().expanded(2.5f),8);
    g.fillRoundedRectangle(modEnvelope.toFloat().expanded(2.5f),8);
    g.fillRoundedRectangle(lfoArea.toFloat().expanded(2.5f),8);

    juce::Rectangle<int> ampLabel = ampArea.removeFromBottom(15).removeFromRight(40).reduced(2);
    juce::Rectangle<int> modLabel = modEnvelope.removeFromBottom(15).removeFromRight(40).reduced(2);
    juce::Rectangle<int> lfoLabel = lfoArea.removeFromBottom(15).removeFromRight(40).reduced(2);
    g.setFont(juce::Font(juce::FontOptions("Montserrat",12,1)));
    g.setColour(juce::Colours::white);
    g.drawText("AMP",ampLabel,juce::Justification::centred);
    g.drawText("MOD",modLabel,juce::Justification::centred);
    g.drawText("LFO",lfoLabel,juce::Justification::centred);


   
}

void Envelope::resized()
{
    constexpr int sliderWidth = 40;
    constexpr int sliderHeight = 115;
    constexpr int margin  = 1;
    constexpr int buttonMargin  = 5;
    constexpr int labelWidth = 25;
    constexpr int labelHeight = 10;
    constexpr int buttonWidth = 40;
    constexpr int buttonHeight = 20;
    constexpr int amountSliderWidth = 20;
    constexpr int amountSliderHeight = 80;
    constexpr int lfoKnobSize =120;
    constexpr int lfoLabelWidth=120;
    constexpr int comboBoxWidth = 110;
    constexpr int comboBoxHeight = 20;
    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> ampArea = area.removeFromLeft(122).reduced(5,5);
    juce::Rectangle<int> modEnvelope = area.removeFromLeft(122).reduced(5);
    juce::Rectangle<int> envelopeButtonsArea = area.removeFromLeft(80).reduced(0,5);
    juce::Rectangle<int> amountSliderArea = envelopeButtonsArea.removeFromRight(20).removeFromTop(sliderHeight);
    juce::Rectangle<int> blank = area.removeFromLeft(5);
    juce::Rectangle<int> lfoArea = area.removeFromLeft(150).reduced(0,5);
    juce::Rectangle<int> lfoKnobsArea = lfoArea.removeFromLeft(80);


    juce::FlexBox amp;
    amp.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(amp,attackAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,decayAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,sustainAmp,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(amp,releaseAmp,sliderWidth,sliderHeight,margin);
    amp.performLayout(ampArea);

    juce::FlexBox mod;
    mod.flexDirection = juce::FlexBox::Direction::row;
    addItemToFlexBox(mod,attackMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,decayMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,sustainMod,sliderWidth,sliderHeight,margin);
    addItemToFlexBox(mod,releaseMod,sliderWidth,sliderHeight,margin);
    mod.performLayout(modEnvelope);


    juce::FlexBox modControls;
    modControls.flexDirection = juce::FlexBox::Direction::column;
    modControls.alignContent = juce::FlexBox::AlignContent::spaceAround;
    addItemToFlexBoxWithAlign(modControls,modEnvType,buttonWidth,buttonHeight,buttonMargin);
    addItemToFlexBoxWithAlign(modControls,loopEnvelope,buttonWidth,buttonHeight,buttonMargin);
    addItemToFlexBox(modControls,sharedAmp,buttonWidth,buttonHeight+10,buttonMargin);
    modControls.performLayout(envelopeButtonsArea);
    modAmount.setBounds(amountSliderArea.withSizeKeepingCentre(50,sliderHeight));

    juce::FlexBox lfoKnobs;
    lfoKnobs.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(lfoKnobs,lfoDepth, lfoKnobSize,lfoKnobSize,margin);
    addItemToFlexBox(lfoKnobs,lfoFreq,lfoKnobSize,lfoKnobSize,margin);
    lfoKnobs.performLayout(lfoKnobsArea.reduced(5,5));

    juce::FlexBox lfoControls;
    lfoControls.flexDirection = juce::FlexBox::Direction::column;
    addItemToFlexBox(lfoControls,lfoNumber,comboBoxWidth,comboBoxHeight,margin);
    addItemToFlexBox(lfoControls,lfoType,comboBoxWidth,comboBoxHeight,margin);
    addItemToFlexBox(lfoControls,lfoReset,comboBoxWidth,buttonHeight,buttonMargin);
    addItemToFlexBox(lfoControls,lfoUnipolar,comboBoxWidth,buttonHeight,buttonMargin);
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
    bool state2 = true;
    bool state3 = true;

    if(button == &loopEnvelope)
    {
        /*bool newToggleState = !button->getToggleState();
        button->setToggleState(newToggleState, juce::dontSendNotification);

        if(envelope==1)
            audioProcessor.state.getParameter("loopEnvelope")->setValueNotifyingHost(newToggleState ? 1.0f : 0.0f);
        if(envelope==2)
            audioProcessor.state.getParameter("loopEnvelope2")->setValueNotifyingHost(newToggleState ? 1.0f : 0.0f);*/
    }
    if(button==&modEnvType)
    {
        if(!button->getToggleState())
        {
            button->setButtonText("Env 2");
            changeEnvelopes(2);
            envelope = 2;
        }

        else if(button->getToggleState())
        {
            button->setButtonText("Env 1");
            changeEnvelopes(1);
            envelope = 1;
        }

        /*state= button->getToggleState();
        button->setToggleState(!state,juce::dontSendNotification);*/
    }

    if(button==&lfoReset)
    {
    }
    if(button==&lfoUnipolar)
    {

    }
    if(button==&sharedAmp)
    {

    }
}


void Envelope::setUpLFOAttachments(juce::StringArray& id)
{
    lfoDepthAttach = std::make_unique<SliderAttachment>(audioProcessor.state,id[0],lfoDepth.slider);
    lfoFreqAttach = std::make_unique<SliderAttachment>(audioProcessor.state,id[1],lfoFreq.slider);
    lfoUnipolarAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,id[2],lfoUnipolar);
    lfoResetAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,id[3],lfoReset);
    lfoTypeAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.state,id[4],lfoType);
}

void Envelope::setUpLFOKnobs(juce::StringArray& id)
{
    lfoDepth.slider.setValue(audioProcessor.state.getRawParameterValue(id[0])->load());
    lfoFreq.slider.setValue(audioProcessor.state.getRawParameterValue(id[1])->load());
    lfoUnipolar.setToggleState(audioProcessor.state.getRawParameterValue(id[2])->load(),juce::dontSendNotification);
    lfoReset.setToggleState(audioProcessor.state.getRawParameterValue(id[3])->load(),juce::dontSendNotification);
    lfoType.setSelectedId(audioProcessor.state.getRawParameterValue(id[4])->load()+1);

}

void Envelope::setUpModAttachments(juce::StringArray& id)
{
    attackModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[0],attackMod.slider);
    decayModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[1],decayMod.slider);
    sustainModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[2],sustainMod.slider);
    releaseModAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[3],releaseMod.slider);
    modAmountAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state,id[4],modAmount.slider);
    loopEnvelopeAttach = std::make_unique<ButtonAttachment>(audioProcessor.state,id[5],loopEnvelope);
}

void Envelope::setUpModSliders(juce::StringArray& id)
{
    attackMod.slider.setValue(audioProcessor.state.getRawParameterValue(id[0])->load());
    decayMod.slider.setValue(audioProcessor.state.getRawParameterValue(id[1])->load());
    sustainMod.slider.setValue(audioProcessor.state.getRawParameterValue(id[2])->load());
    releaseMod.slider.setValue(audioProcessor.state.getRawParameterValue(id[3])->load());
    modAmount.slider.setValue(audioProcessor.state.getRawParameterValue(id[4])->load());
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
            lfo = 1;
        }
        if(lfoNumber.getSelectedId()==2)
        {
            setUpLFOKnobs(LFO2IDs);
            setUpLFOAttachments(LFO2IDs);
            lfo = 2;
        }
        if(lfoNumber.getSelectedId()==3)
        {
            setUpLFOKnobs(LFO3IDs);
            setUpLFOAttachments(LFO3IDs);
            lfo = 3;
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
