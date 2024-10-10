/*
  ==============================================================================

    LFO.h
    Created: 6 May 2024 8:54:01pm
    Author:  blaze

  ==============================================================================
*/


#pragma once
#include <JuceHeader.h>
class LFO {
public:
    explicit LFO(juce::ValueTree& v):tree(v)
    {
    }
    ~LFO()=default;
    void prepareToPlay(double sampleRate, unsigned int numChannels, unsigned int blockSize)
    {
        reset();
        auto spec = juce::dsp::ProcessSpec{ sampleRate/updateRate,blockSize,numChannels };
        lfo.prepare(spec);
        lfo.initialise([](float x) { return std::sin(x); });

    }
    float render()
    {
        float modFreq = modValue[kFREQ]*parameters.frequency;
        lfo.setFrequency(std::clamp(parameters.frequency+modFreq,0.f,20.f));
        return lfo.processSample(0.0) * std::clamp(parameters.depth+modValue[kAMOUNT],0.f,1.f);
    }
    void reset()
    {
        lfo.reset();
    }
    void setParameters()
    {
        parameters.depth=static_cast<float>(tree[IDs::LFODepth])/100.f;
        parameters.frequency = static_cast<float>(tree[IDs::LFOFreq]);
        parameters.type = static_cast<int>(tree[IDs::LFOType]);
        setLFOType();
        lfo.setFrequency(parameters.frequency);
    }
    float* getModAmount()
    {
        return &modValue[kAMOUNT];
    }
    float* getModFrequency()
    {
        return &modValue[kFREQ];
    }
    void setLFOType()
    {
	    switch ((parameters.type))
	    {
	    case 0:
            lfo.initialise([](float x) { return std::sin(x); });
            break;
	    case 1:
		    // Square wave
		    lfo.initialise([](float x) -> float
		    {
			    return (x < 0) ? -1.0f : 1.0f;
		    });
            break;
	    case 2:
            // Saw wave
            lfo.initialise([](float x) -> float {
                return 2.0f * (x / (2.0f * juce::MathConstants<float>::pi) - floor(0.5f + x / (2.0f * juce::MathConstants<float>::pi)));
                });
            break;
	    default:
            lfo.initialise([](float x) { return std::sin(x); });
            break;
	    }
    }

private:
    enum{kAMOUNT,kFREQ,kNumDest};
    juce::dsp::Oscillator<float> lfo;
    struct LFOParameters
    {
        float frequency{ 0.0f };
        float depth{ 0.0f };
        float delay{ 0.0f };
        int type{ 0 };
        float modValue{ 0.0f };
    };
    std::array<float,kNumDest> modValue{0.0f};
    LFOParameters parameters;
    juce::ValueTree tree;
    int modDest{ 0 };
    int updateRate{ 32 };
    int updateCounter{ updateRate };
};
