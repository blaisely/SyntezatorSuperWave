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
    setSize (600, 600);

    addAndMakeVisible(filterGui);
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
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
   
}

void SimpleSynthAudioProcessorEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> topSection = getLocalBounds().removeFromTop(400);
    juce::Rectangle<int> topSection2 = getLocalBounds().removeFromTop(400);
    juce::Rectangle<int> bottomSection = getLocalBounds().removeFromBottom(200);
    

    oscGui.setBounds(topSection.removeFromLeft(400));
    filterGui.setBounds(topSection2.removeFromRight(200));
    envelopeGui.setBounds(bottomSection.removeFromLeft(200));
   
   
}



