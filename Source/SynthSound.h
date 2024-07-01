/*
  ==============================================================================

    SourceSound.h
    Created: 8 Apr 2024 3:21:55pm
    Author:  blaze

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Osc.h"


class SynthSound : public juce::SynthesiserSound
{
public:

    bool appliesToChannel(int midiChannel) {
        return true;
    }
    bool appliesToNote(int midiNoteNumber) {
        return true;
    }


    

};