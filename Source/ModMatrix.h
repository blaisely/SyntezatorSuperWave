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
        kLFO3,
        kAMP,
        kEG,
        kEG2,
        kNumSrc
    };
    enum modDestination
    {
        kFILTER_CUTOFFSVF,
        kFILTER_CUTOFFLDDR,
        kFILTER_RESONANCE,
        kOSC_VOLUME,
        kOSC_DETUNE,
        kOSC1_PITCH,
        kOSC2_PITCH,
        kOSC1_GAIN,
        kOSC2_GAIN,
        kOSC1_PAN,
        kOSC2_PAN,
        kLFO1_AMT,
        kLFO2_AMT,
        kLFO1_FREQ,
        kLFO2_FREQ,
        kOSC1_PWM,
        kOSC2_PWM,
        kOSC1_TYPE,
        kOSC2_TYPE,
        kNumDest
    };
    struct ModDest
    {
        float* value=nullptr;
        std::array<float,kNumSrc> intensity={0.0f};
        bool isEnabled = false;
    };
    struct ModSource
    {
        float* value=nullptr;
    };
    ModMatrix();
    void addDestination(int destination,float* value );
    void addSource(int source, float* value);
    void addRouting(int source, int destination,float intensity);
    void resetRouting(int source, int destination);
    void render();
private:
    std::array<ModDest,kNumDest> destinations;
    std::array<ModSource,kNumSrc> sources;
};



#endif //MODMATRIX_H
