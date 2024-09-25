//
// Created by blaze on 24.09.2024.
//

#ifndef MODMATRIX_H
#define MODMATRIX_H
#include <array>
#include <vector>
#include "SharedData.h"


class ModMatrix {
public:
    enum modSource
    {
        kLFO,
        kLFO2,
        kAMP,
        kMOD_EG,
        NOTE,
        VELOCITY,
        numSrc
    };
    enum modDestination
    {
        kFILTER_CUTOFF,
        kRESONANCE,
        kOSC1_GAIN,
        kDETUNE,
        kVOLUME,
        kGAIN_OSC1,
        kGAIN_OSC2,
        kAMP_ATT,
        kAMP_DEC,
        kAMP_SUS,
        kAMP_REL,
        kDEST_INTENSITY,
        kLFO_FREQUENCY,
        kEG_ATT,
        kEG_DEC,
        kEG_SUS,
        kEG_REL,
        kLFO2_FREQUENCY,
        kFILTER_DRIVE,
        numDest
    };
    ModMatrix(juce::ValueTree& v);
    void addDestinations(int id,juce::Identifier destination);
    void addSource(int id,float* source);
    void setRouting(enum modSource s,enum modDestination d,float intensity);
    void render();
private:

    struct modulation
    {
        modSource sourceID;
        modDestination destinationID;
        float* inputValue;
        juce::Identifier output;
        float intensity;
    };
    std::array<juce::Identifier,numDest> modDestination  {nullptr};
    std::array<float*,numSrc> modSource  {nullptr};
    std::vector<modulation> modulations;
    juce::ValueTree tree;
};



#endif //MODMATRIX_H
