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
        float* value=nullptr;
        bool isEnabled;
    };
    struct ModSource
    {
        float* value=nullptr;
    };
    enum modSource
    {
        kLFO,
        kEG,
        kNumSrc
    };
    enum modDestination
    {
        kFILTER_CUTOFF,
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
    std::array<float,kNumSrc> intensity{0.0f};
};



#endif //MODMATRIX_H
