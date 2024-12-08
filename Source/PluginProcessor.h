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
class SuperWaveSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SuperWaveSynthAudioProcessor();
    ~SuperWaveSynthAudioProcessor() override;

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
    static void resetAllParameters(juce::AudioProcessorValueTreeState& s);
    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    struct chainSettings {
        float cutoff{ 1000.0f }, resonance{ 0.0f }, oscType_osc1{ 0.0f }, oscType_osc2{ 0.0f }, filterType{ 0 }, attack{ 0.0f }, decay{ 0.0f }, sustain{ 0.0f },
            release{ 0.0f }, gain_osc1{ 0.0f }, gain_osc2{ 0.0f },lfofreq{0.0f}, lfodepth{0.0f}, detune{0.0f}, volume{0.0f},
        attackEnv2{0.0f},decayEnv2{0.0f},sustainEnv2{0.0f},releaseEnv2{0.0f},envelopeAmount{0.0f},attackEnv3{0.0f},decayEnv3{0.0f},sustainEnv3{0.0f},
        releaseEnv3{0.0f},panOsc1{0.0f},panOsc2{0.0f},gainOVR{0.0f},
        lfo2freq{0.0f},lfo2depth{0.0f},modIntensity1{0.f},modIntensity2{0.f},modIntensity3{0.f},modIntensity4{0.f},modIntensity5{0.f},modIntensity6{0.f},pulseWidthOsc1{0.0f},
        pulseWidthOsc2{0.0f},filterVelocity{0.0f},lfo3freq{0.0f},lfo3depth{0.0f},envelopeAmount2{0.f};
        
        int octaveosc1{ 0 }, octaveosc2{ 0 }, coarseosc1{0}, coarseosc2{ 0},filterOn{0},filterDrive{0},commonEnvelope{0},
        lfoReset{0},lfoType{0},reversedEnvelope{0},detuneosc1{0},detuneosc2{0},lfo2Type{0},modSource1{0},modSource2{0},modSource3{0},
        modSource4{0},modSource5{0},modSource6{0},modDestination1{0},modDestination2{0},modDestination3{0},modDestination4{0},modDestination5{0},modDestination6{0},loopModEnvelope{0},
        lfo1Unipolar{0},lfo2Unipolar{0},filterKeytrack{0},filterKeytrackOffset{0},lfo3Type{0},lfo3Unipolar{0},loopModEnvelope2{0},
        lfoReset2{0},lfoReset3{0},reset{0},lfoNumber{1}, aliasingON{0};
    };
    static chainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
    static juce::ValueTree createValueTree();
    static void syncStates(juce::ValueTree& tree,chainSettings& s);

private:
    juce::ValueTree tree;
    juce::Synthesiser mySynth;
    SynthVoice* myVoice;
    float lastSampleRate;
    bool resetSynth{false};
    juce::dsp::Gain<float> pluginGain;
    juce::SmoothedValue<float> gainAmt{0.0f};
    float sampleRate{48000.f};
    void reset() override;
    float* pointer = nullptr;
     juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  DCOffset;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperWaveSynthAudioProcessor)
};
