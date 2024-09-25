//
// Created by blaze on 24.09.2024.
//

#include "ModMatrix.h"

ModMatrix::ModMatrix(juce::ValueTree& v):tree(v)
{
}
void ModMatrix::addSource(const int id, float* source)
{
    modSource[id]=source;
}

void ModMatrix::addDestinations(const int id, juce::Identifier destination)
{
  modDestination[id]=destination;
}
void ModMatrix::render()
{
    for(auto& m : modulations)
    {
        float dest = tree[m.output];
        dest += *m.inputValue * m.intensity;
        tree.setProperty(m.output,dest,nullptr);
    }

}
void ModMatrix::setRouting(enum modSource s, enum modDestination d, const float intensity)
{

    modulations.push_back({s,d,modSource[s],modDestination[d],intensity});
}



