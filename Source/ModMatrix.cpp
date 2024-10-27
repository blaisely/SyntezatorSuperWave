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

void ModMatrix::addRouting(int source, int destination, float modIntensity)
{

        DBG("Mod Matrix set to Source: "+std::to_string(source) + " and Destination: "+std::to_string(destination));
        destinations[destination].intensity[source] = modIntensity;
        destinations[destination].isEnabled = true;

}

void ModMatrix::resetRouting(int source, int destination)
{
    DBG("Mod Matrix reset to Source: "+std::to_string(source) + " and Destination: "+std::to_string(destination));
        destinations[destination].intensity[source]=0.0f;
    float rest {0.0f};
    *destinations[destination].value = rest;
}

void ModMatrix::render()
{
    //Go through every row and column and sum the modulation values
    float modDestinationValue=0.0f;
    for(auto column=0;column<kNumDest;column++)
    {
        for(int row=0;row<kNumSrc;row++)
        {
            if(!destinations[column].isEnabled || destinations[column].intensity[row]==0.0f || sources[row].value==nullptr)
                continue;

            float modSourceValue = *sources[row].value;

            modDestinationValue+=destinations[column].intensity[row] * modSourceValue;

            if(destinations[column].isEnabled)
                *destinations[column].value=modDestinationValue;
        }

    }
}







