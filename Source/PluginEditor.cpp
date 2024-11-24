/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include "BinaryData.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SuperWaveSynthAudioProcessorEditor::SuperWaveSynthAudioProcessorEditor (SuperWaveSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), filterGui(p), oscGui(p), envelopeGui(p),matrixGui(p),presetGui(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (840, 500);
    logo = juce::ImageCache::getFromMemory(BinaryData::SUPERWAVE_logo_png,BinaryData::SUPERWAVE_logo_pngSize);
    if(!logo.isValid())
        DBG("Image Invalid");
    addAndMakeVisible(oscGui);
    addAndMakeVisible(envelopeGui);
    addAndMakeVisible(filterGui);
    addAndMakeVisible(matrixGui);
    addAndMakeVisible(presetGui);

   
}

SuperWaveSynthAudioProcessorEditor::~SuperWaveSynthAudioProcessorEditor()
{
}

//==============================================================================
void SuperWaveSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xffD1CAF1));
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> nameArea = area.removeFromRight(100).removeFromTop(295).reduced(20,15);
    g.setColour(juce::Colour(0xff949FD6));
    g.fillRoundedRectangle(nameArea.toFloat(),8);
    juce::AffineTransform rot;

    g.drawImage(logo,nameArea.toFloat(),true,false);

    /*g.saveState();

    // Calculate the center of the nameArea to use as the rotation pivot
    float centerX = static_cast<float>(nameArea.getCentreX());
    float centerY = static_cast<float>(nameArea.getCentreY());

    g.setColour(juce::Colours::whitesmoke);
    g.setFont(juce::Font(juce::FontOptions("Montserrat",20.0f,1)));
    // Move the origin to the center of the nameArea
    g.addTransform(juce::AffineTransform::translation(centerX, centerY));

    // Rotate by 90 degrees clockwise
    g.addTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::halfPi));

    // Move the origin back to its original position
    g.addTransform(juce::AffineTransform::translation(-centerX, -centerY));
    
    g.setColour(juce::Colours::whitesmoke);
    g.setFont(juce::Font(juce::FontOptions("Montserrat",20.0f,1)));
    // Draw the text centered in the rotated nameArea
    g.drawFittedText("SUPER WAVE", nameArea.getX(), nameArea.getY(),
                    nameArea.getWidth(), nameArea.getHeight(), juce::Justification::topLeft, 0);


    // Restore the previous graphics context state
    g.restoreState();*/
   
}

void SuperWaveSynthAudioProcessorEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    juce::Rectangle<int> top = area.removeFromTop(295);
    juce::Rectangle<int> topSection = top.removeFromLeft(530);
    juce::Rectangle<int> leftSection = top.removeFromLeft(220);
    juce::Rectangle<int> bottomSection = area.removeFromBottom(200);
    juce::Rectangle<int> leftBottomSection = bottomSection.removeFromLeft(530);
    juce::Rectangle<int> matrixSection = bottomSection.removeFromLeft(300);

    juce::Rectangle<int> controlsSection  = leftBottomSection.removeFromTop(160);
    juce::Rectangle<int> presetSection = leftBottomSection;

    oscGui.setBounds(topSection.reduced(15));
    envelopeGui.setBounds(controlsSection.reduced(15,0));
    filterGui.setBounds(leftSection.reduced(10,15));
    matrixGui.setBounds(matrixSection.reduced(5));
    presetGui.setBounds(leftBottomSection.reduced(10,0));

}



