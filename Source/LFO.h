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
        auto spec = juce::dsp::ProcessSpec{ sampleRate,blockSize,numChannels };
        lfo.prepare(spec);
        lfo.initialise([](float x) { return std::sin(x); });
    }
    float render()
    {
        float modFreq = modValue[kFREQ]*parameters.frequency;
        lfo.setFrequency(std::clamp(parameters.frequency+modFreq,0.f,20.f));
    	if(parameters.isUnipolar)
    		return (lfo.processSample(0.0) * std::clamp(parameters.depth+modValue[kAMOUNT],0.f,1.f))+1.f*0.5f;
    	else
    		return (lfo.processSample(0.0) * std::clamp(parameters.depth+modValue[kAMOUNT],0.f,1.f));

    }
    void reset()
    {
        lfo.reset();
    }
    void setParameters(const float depth, const float freq, const int type,const bool isUnipolar)
    {
        parameters.depth= depth;
        parameters.frequency = freq;
        parameters.type = type;
    	parameters.isUnipolar = isUnipolar;
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
	    case 3:
	        lfo.initialise([](float x)->float
	        {
	           static float lastValue = 0.0f;
	            static float lastPhase = 0.0f;
	        	if(x<0.0f&&lastPhase>= 0.0f)
	        	{
	        		lastValue = juce::Random::getSystemRandom().nextFloat()*2.0f-1.0f;
	        	}
	        	lastPhase = x;
	        	return lastValue;
	        	//at every zero crossing generate new random value and hold it
	        });
	        break;
	    case 4:
	    	lfo.initialise([](float x)->float
	    	{
	    		static float currentValue=0.0f;
	    		static float targetValue = 0.0f;
	    		static float interpolation = 0.0f;
	    		static float lastPhase = 0.0f;
	    		if(x<0.0f && lastPhase>=0.0f)
	    		{
	    			targetValue = juce::Random::getSystemRandom().nextFloat()*2.0f-1.0f;
	    			interpolation = 0.0f;
	    		}
	    		interpolation+=0.0001f;
	    		if(interpolation>1.0f)
	    				interpolation = 1.0f;
	    		currentValue = juce::jmap(interpolation,0.f,1.0f,currentValue,targetValue);
	    		lastPhase = x;
	    		return currentValue;

	    	});
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
    	bool isUnipolar{false};
    };
    std::array<float,kNumDest> modValue{0.0f};
    LFOParameters parameters;
    juce::ValueTree tree;
    int modDest{ 0 };
    int updateRate{ 32 };
    int updateCounter{ updateRate };
};
