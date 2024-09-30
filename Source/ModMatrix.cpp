//
// Created by blaze on 24.09.2024.
//

#include "ModMatrix.h"

#include <utility>

ModMatrix::ModMatrix()
{
}

void ModMatrix::addDestination(int destination, float* value)
{
    destinations[destination].value = value;
}

void ModMatrix::addSource(int source, float* value)
{
    sources[source].value = value;
}

void ModMatrix::addRouting(int source, int destination, float intensity)
{
    destinations[destination].intensity = intensity;
    destinations[destination].isEnabled = true;
}
void ModMatrix::render()
{
    float modDestinationValue=0.0f;
    for(auto column=0;column<kNumDest;column++)
    {
        for(auto row=0;row<kNumSrc;row++)
        {
            if(!destinations[row].isEnabled)
                continue;

            float modSourceValue = *sources[row].value;

            modDestinationValue+=destinations[row].intensity * modSourceValue;
        }
        if(destinations[column].isEnabled)
            *destinations[column].value=modDestinationValue;
    }
}







