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
    void prepareToPlay(double sampleRate, unsigned int numChannels, unsigned int blockSize)
    {
        this->sampleRate = sampleRate;
        this->numChannels = numChannels;
        this->blockSize = blockSize;
        auto spec = juce::dsp::ProcessSpec{ sampleRate/updateRate,blockSize,numChannels };
        
        lfo1.prepare(spec);
        reset();
       
        lfo1.initialise([](float x) { return std::sin(x); });
	        
    }
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample,int numSamples)
    {
       
        for(size_t pos =0;pos<(size_t)numSamples;)
        {
            auto max = juce::jmin((size_t)(numSamples) - pos, (size_t)updateCounter);
            pos += max;
            updateCounter -= max;
            if(updateCounter==0)
            {
                updateCounter = updateRate;
                paramsLfo1.modValue = lfo1.processSample(0.0) * paramsLfo1.depth;
                
            }
        }


        //for (auto i = 0; i < blockSize; ++i) {
        //    paramsLfo1.modValue = lfo1.processSample(0.0f) * paramsLfo1.depth;
        //    //paramsLfo2.modValueLfo2 = lfo2.processSample(0.0f) * paramsLfo2.depthLfo2;
        //   
        //}
    }
    void reset()
    {
        lfo1.reset();
        //lfo2.reset();
    }
    void getParametersLfo1(float* frequencyLfo1,float* depthLfo1,float&& typeLfo1,float&& delayLfo1)
    {
        paramsLfo1.frequency = *frequencyLfo1;
        paramsLfo1.depth = *depthLfo1;
        paramsLfo1.type = typeLfo1;
        paramsLfo1.delay = delayLfo1;
        setLfo1Type();
        lfo1.setFrequency(paramsLfo1.frequency);
    }
   /* void getParametersLfo2(const float frequencyLfo2, const float depthLfo2, const float typeLfo2, const float delayLfo2)
    {
        paramsLfo2.frequencyLfo2 = frequencyLfo2;
        paramsLfo2.depthLfo2 = depthLfo2;
        paramsLfo2.typeLfo2 = typeLfo2;
        paramsLfo2.delayLfo2 = delayLfo2;
        setLfo2Type();
    }*/
    void setLfo1Type()
    {
	    switch (static_cast<int>(paramsLfo1.type))
	    {
	    case 0:
            lfo1.initialise([](float x) { return std::sin(x); });
            break;
	    case 1:
		    // Square wave
		    lfo1.initialise([](float x) -> float
		    {
			    return (x < 0) ? -1.0f : 1.0f;
		    });
            break;
	    case 2:
            // Saw wave
            lfo1.initialise([](float x) -> float {
                return 2.0f * (x / (2.0f * juce::MathConstants<float>::pi) - floor(0.5f + x / (2.0f * juce::MathConstants<float>::pi)));
                });
            break;
	    default:
            lfo1.initialise([](float x) { return std::sin(x); });
            break;
	    }
    }

    decltype(auto) getModValueLfo1()
    {
       return paramsLfo1.modValue;
    }
   
private:
    juce::dsp::Oscillator<float> lfo1;
   

    struct LFOParameters
    {
        float frequency{ 0.0f };
        float depth{ 0.0f };
        float delay{ 0.0f };
        float type{ 0.0f };
        float modValue{ 0.0f };
        
    };
    LFOParameters paramsLfo1;
   
    int sampleRate{ 0 };
    int numChannels{ 0 };
    int modDest{ 0 };
    int updateRate{ 100 };
    int updateCounter{ updateRate };
    int blockSize{ 0 };
};
