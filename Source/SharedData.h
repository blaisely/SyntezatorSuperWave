/*
  ==============================================================================

    SharedData.h
    Created: 2 Jun 2024 11:29:22pm
    Author:  blaze

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


using ID = const juce::Identifier;
namespace IDs{

    const juce::Identifier Parameters("Parameters");
    ID Oscillator("Oscillator");
    ID VAOsc("VAOscillator");
    ID SuperWaveOsc("SuperWaveOsc");
    ID Cutoff("Cutoff");
    ID Resonance("Resonance");
    ID ADSR1Attack("adsr1_a");
    ID ADSR1Decay("adsr1_d");
    ID ADSR1Sustain("adsr1_s");
    ID ADSR1Release("adsr1_r");
    ID ADSR2Attack("adsr2_a");
    ID ADSR2Decay("adsr2_d");
    ID ADSR2Sustain("adsr2_s");
    ID ADSR2Release("adsr2_r");
    ID ADSR2Mod("adsr2_mod");
    ID LFODepth("LFODepth");
    ID LFO2Depth("LFO2Depth");
    ID LFOFreq("LFOFrequency");
    ID LFO2Freq("LFO2Frequency");
    ID LFOType("LFOType");
    ID LFO2Type("LFO2Type");
    ID LFOMod("LFOMod");
    ID Filter("Filter");
    ID FilterT("FilterType");
    ID SWtype("Oscillator1Type");
    ID VAtype("VirtualAnalogType");
    ID SWoctave("SuperWaveOctave");
    ID VAoctave("VirualAnalogOctave");
    ID SWdetune("SuperWaveDetune");
    ID VAdetune("VirualAnalogDetune");
    ID SWdetuneS("SuperWaveDetuneS");
    ID SWvolumeS("SuperWaveVolumeS");
    ID VAgain("VirtualAnalogGain");
    ID SWgain("SuperWaveGain");
    ID LFO("Lfo");
    ID ADSR1("ADSR1");
    ID ADSR2("ADSR2");
    ID SampleRate("SmpRate");
    ID GainOvr("GainOvr");
    ID SWFrequency("SWFreq");
    ID VAFrequency("VAFreq");
    ID MidiNote("MidiNote");
    ID SVFEnabled("SVF Enabled");
    ID FilterDrive("Filter Drive");
    ID LFOReset("LFO Reset");
    ID CommonEnvelope("Common Envelope");
    ID FilterEnvelopeAmount("Filter Envelope Amount");
    ID ReversedEnvelope("Reversed Mod Envelope");
    ID PanOsc1("Pan OSC1");
    ID PanOsc2("Pan OSC2");
    ID SWCoarse("Coarse SW");
    ID VACoarse("Coarse VA");
    ID ModSource1("Mod Source1");
    ID ModSource2("Mod Source2");
    ID ModSource3("Mod Source3");
    ID ModSource4("Mod Source4");
    ID ModDestination1("Mod Destination1");
    ID ModDestination2("Mod Destination2");
    ID ModDestination3("Mod Destination3");
    ID ModDestination4("Mod Destination4");
    ID ModIntensity1("Mod Intensity1");
    ID ModIntensity2("Mod Intensity2");
    ID ModIntensity3("Mod Intensity3");
    ID ModIntensity4("Mod Intensity4");
    ID LoopEnvelope("Loop Mod Envelope");
    ID PulseWidthOSC1("Pulse Width OSC1");
    ID PulseWidthOSC2("Pulse Width OSC2");
    ID LFO1Unipolar("LFO1 Unipolar");
    ID LFO2Unipolar("LFO2 Unipolar");


};
