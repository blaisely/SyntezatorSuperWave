/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthVoice.h"
#include "SynthSound.h"
#include "SharedData.h"
//==============================================================================
/**
*/
class SimpleSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleSynthAudioProcessor();
    ~SimpleSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    void updateFilter();
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void IIRfilter(float frequency, float sampleRate, float* samples, float numSamples);

    float cutOff;
    float resonance;
    

    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    struct chainSettings {
        float cutoff{ 1000.0f }, resonance{ 0.0f }, oscType_osc1{ 0 }, oscType_osc2{ 0.0f }, filterType{ 0 }, attack{ 0.0f }, decay{ 0.0f }, sustain{ 0.0f },
            release{ 0.0f }, gain_osc1{ 0.0f }, gain_osc2{ 0.0f },lfofreq{0.0f}, lfodepth{0.0f}, detune{0.0f}, volume{0.0f},
        attackEnv2{0.0f},decayEnv2{0.0f},sustainEnv2{0.0f},releaseEnv2{0.0f},envelopeAmount{0.0f},panOsc1{0.0f},panOsc2{0.0f};
        
        int octaveosc1{ 0 }, octaveosc2{ 0 }, coarseosc1{0}, coarseosc2{ 0},filterOn{0},filterDrive{0},commonEnvelope{0},
        lfoReset{0},lfoType{0},reversedEnvelope{0};
    };
    static chainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

    
    static juce::ValueTree createValueTree();
    static void syncStates(juce::ValueTree& tree,chainSettings& s);
private:
    juce::ValueTree tree;
    juce::Synthesiser mySynth;
    SynthVoice* myVoice;
    float lastSampleRate;
    void reset() override;

   
     float s1;
     float s2;
     float Q = 1.0f;

     juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  lowPassFilter;
     juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  DCOffset;

     
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSynthAudioProcessor)
};
