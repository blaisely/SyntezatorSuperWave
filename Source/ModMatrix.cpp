//
// Created by blaze on 24.09.2024.
//

#include "ModMatrix.h"

#include <utility>

ModMatrix::ModMatrix(juce::ValueTree& v):tree(v)
{
}

void ModMatrix::render()
{
    for(auto& m : modulations)
    {
        float dest = tree[m.output];
        dest += *m.inputValue * m.intensity;
        juce::Logger::outputDebugString("Source Value: "+std::to_string(*m.inputValue));
        juce::Logger::outputDebugString("Destination Value: "+std::to_string(dest));
        tree.setProperty(m.output,dest,nullptr);
    }

}
void ModMatrix::setRouting(enum modSource s, enum modDestination d, const float intensity,float* sourceValue,juce::Identifier dstID)
{

    modulations.push_back({s,d,sourceValue,dstID,intensity});
}



