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
        kEG,
        kNumSrc
    };
    enum modDestination
    {
        kFILTER_CUTOFFSVF,
        kFILTER_CUTOFFLDDR,
        kOSC_DETUNE,
        kOSC_VOLUME,
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
