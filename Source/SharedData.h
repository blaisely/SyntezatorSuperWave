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
    ID LFOFreq("LFOFrequency");
    ID LFOType("LFOType");
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
    ID SWGainOvr("SWGainOvr");
    ID VAGainOvr("VAGainOvr");
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


};
