/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSynthAudioProcessorEditor::SimpleSynthAudioProcessorEditor (SimpleSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), filterGui(p), oscGui(p), envelopeGui(p),matrixGui(p),presetGui(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (840, 500);


    addAndMakeVisible(oscGui);
    addAndMakeVisible(envelopeGui);
    addAndMakeVisible(filterGui);
    addAndMakeVisible(matrixGui);
    addAndMakeVisible(presetGui);

   
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
    juce::Rectangle<int> top = area.removeFromTop(295);
    juce::Rectangle<int> topSection = top.removeFromLeft(530);
    juce::Rectangle<int> leftSection = top.removeFromRight(300);
    juce::Rectangle<int> bottomSection = area.removeFromBottom(200);
    juce::Rectangle<int> matrixSection = bottomSection.removeFromRight(300);
    juce::Rectangle<int> leftBottomSection = bottomSection.removeFromLeft(530);
    juce::Rectangle<int> controlsSection  = leftBottomSection.removeFromTop(160);
    juce::Rectangle<int> presetSection = leftBottomSection;


    

    oscGui.setBounds(topSection.reduced(15));
    envelopeGui.setBounds(controlsSection.reduced(15,0));
    filterGui.setBounds(leftSection.reduced(5).reduced(20,0));
    matrixGui.setBounds(matrixSection.reduced(5));
    presetGui.setBounds(leftBottomSection.reduced(10,5));

}



