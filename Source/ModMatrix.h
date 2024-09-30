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
    struct ModDest
    {
        float* value;
        float intensity;
        bool isEnabled;
    };
    struct ModSource
    {
        float* value;
    };
    enum modSource
    {
        kLFO,
        kLFO2,
        kAMP,
        kMOD_EG,
        NOTE,
        VELOCITY,
        kNumSrc
    };
    enum modDestination
    {
        kFILTER_CUTOFF,
        kRESONANCE,
        kGAIN_OSC1,
        kNumDest
    };
    ModMatrix();
    void addDestination(int destination,float* value );
    void addSource(int source, float* value);
    void addRouting(int source, int destination,float intensity);
    void render();
private:
    std::array<ModDest,kNumDest> destinations;
    std::array<ModSource,kNumSrc> sources;
};



#endif //MODMATRIX_H
