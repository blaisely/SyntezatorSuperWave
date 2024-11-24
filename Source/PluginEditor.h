/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "oscGUI.h"
#include "filterGUI.h"
#include "Envelope.h"
#include "modMatrixGUI.h"
#include "presetGUI.h"
//==============================================================================
/**
*/
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
class SuperWaveSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SuperWaveSynthAudioProcessorEditor (SuperWaveSynthAudioProcessor&);
    ~SuperWaveSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    

private:
    SuperWaveSynthAudioProcessor& audioProcessor;
    juce::Image logo;
    oscGUI oscGui;
    filterGUI filterGui;
    Envelope envelopeGui;
    modMatrixGUI matrixGui;
    presetGUI presetGui;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperWaveSynthAudioProcessorEditor)
};
