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
    float render(int startSample,int numSamples)
    {
       
        for(size_t pos =0;pos<(size_t)numSamples;)
        {
            const auto max = juce::jmin((size_t)(numSamples) - pos, (size_t)updateCounter);
            pos += max;
            updateCounter -= max;
            if(updateCounter==0)
            {
                updateCounter = updateRate;
                float lfoMod = lfo.processSample(0.0) * parameters.depth;
                float currentCutoff = tree[IDs::Cutoff];
                filterZip += 0.005f * (lfoMod - filterZip);
                float modulatedCutoff = currentCutoff * filterZip;
                modulatedCutoff+=currentCutoff;
                modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
                return modulatedCutoff;
            }
        }
        return 0.0f;

    }
    void reset()
    {
        lfo.reset();
    }
    void setParameters()
    {
        parameters.depth=static_cast<float>(tree[IDs::LFODepth])/100.f;
        parameters.frequency = static_cast<float>(tree[IDs::LFOFreq])/static_cast<float>(updateRate);
        parameters.type = static_cast<int>(tree[IDs::LFOType]);
        setLFOType();
        lfo.setFrequency(parameters.frequency);
    }
    void setLFOType()
    {
	    switch (static_cast<int>(parameters.type))
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
    juce::dsp::Oscillator<float> lfo;

    struct LFOParameters
    {
        float frequency{ 0.0f };
        float depth{ 0.0f };
        float delay{ 0.0f };
        int type{ 0 };
        float modValue{ 0.0f };
        
    };
    float filterZip{0.0f};
    LFOParameters parameters;
    juce::ValueTree tree;
    int modDest{ 0 };
    int updateRate{ 100 };
    int updateCounter{ updateRate };
};
