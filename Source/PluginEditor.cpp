/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSynthAudioProcessorEditor::SimpleSynthAudioProcessorEditor (SimpleSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), filterGui(p), oscGui(p), envelopeGui(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (850, 465);

    //addAndMakeVisible(filterGui);
    addAndMakeVisible(oscGui);
    addAndMakeVisible(envelopeGui);

   
}

SimpleSynthAudioProcessorEditor::~SimpleSynthAudioProcessorEditor()
{
}

//==============================================================================
void SimpleSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colour(0xffC9C7EE));
    juce::Rectangle<int> area = getLocalBounds();

    /*juce::Rectangle<int> bottomSection = area.removeFromBottom(150);
    g.setColour(juce::Colour(0xff312F2F));
    g.fillRoundedRectangle(bottomSection.reduced(5).toFloat(),6);*/
    g.setFont(15.0f);
   
}

void SimpleSynthAudioProcessorEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> topSection = area.removeFromTop(275).removeFromLeft(530);
    juce::Rectangle<int> bottomSection = area.removeFromBottom(190).removeFromLeft(530);
    juce::Rectangle<int> controlsSection  = bottomSection.removeFromTop(150);
    

    oscGui.setBounds(topSection.reduced(5));
    envelopeGui.setBounds(controlsSection.reduced(5));
   
   
}



