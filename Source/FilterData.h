/*
  ==============================================================================

    FilterData.h
    Created: 22 Apr 2024 1:02:44pm
    Author:  blaze

  ==============================================================================
*/

#pragma once
#include <complex>
#include <cmath>
#include <JuceHeader.h>


class Filter: public juce::dsp::StateVariableTPTFilter<float>,public juce::ValueTree::Listener
{
public:
    Filter(juce::ValueTree v):tree(v)
    {
        tree.addListener(this);
    }
    ~Filter()
    {
        tree.removeListener(this);
    }
	void setFilterParam(float cutOffFreq, float res, float type)
	{
		cutOff = cutOffFreq;
        resonance = res;
		f_type = type;
	}

	void setFType()
	{
		switch(static_cast<int>(f_type))
		{
		case 0:
			setType(juce::dsp::StateVariableTPTFilterType::lowpass);
			break;
		case 1:
			setType(juce::dsp::StateVariableTPTFilterType::highpass);
			break;
		case 2:
			setType(juce::dsp::StateVariableTPTFilterType::bandpass);
			break;
		default:
			setType(juce::dsp::StateVariableTPTFilterType::lowpass);
			break;
		}
	}
	void setClampedCutOff(float lfoMod)
	{
        float minCutoff = 20.0f; // Example minimum cutoff frequency
        float maxCutoff = 20000.0f; // Example maximum cutoff frequency
        float logMin = std::log10(minCutoff);
        float logMax = std::log10(maxCutoff);

        // Scale LFO value to the logarithmic range and convert back to linear frequency
        float logCutoff = juce::jmap(lfoMod, -1.0f, 1.0f, 20.0f, 20000.0f);
        float cutoffFrequency = std::pow(10.0f, logCutoff);
		setCutoffFrequency(cutOff);
		setResonance(resonance);
		setFType();
	}

	void prepareToPlay(const double sampleRate,const int samplesPerBlock,const int outputChannels)
	{
		resetAll();
		juce::dsp::ProcessSpec spec;
		spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = outputChannels;
		prepare(spec);
		//lfo.prepare(spec);*/
	}
	void resetAll()
	{
		reset();
		//lfo.reset();
	}
	void processNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
	{	
		juce::dsp::AudioBlock<float> block {buffer};
		process(juce::dsp::ProcessContextReplacing<float>(block));
		
	}
	void processNextAudioSample(const float input,const int channel)
	{
		processSample(channel, input);
	}
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
    {
        
        if(treeWhosePropertyHasChanged==tree)
        {
            DBG("Cutoff Moved!");
            setFilterParam(tree[IDs::Cutoff], tree[IDs::Resonance], f_type);
        }
	    
	    
    }


private:
	float cutOffMod{ 0.0f };
	float cutOff{ 0.0f };
	float resonance{ 0.1f };
	float f_type{ 0.0f };
    juce::ValueTree tree;
};
class StateVariableFilter
{
public:
    StateVariableFilter() : m0(0.0f), m1(0.0f), m2(0.0f) { }

    void setCoefficients(double sampleRate, double freq, double Q) noexcept
    {
        g = std::tan(juce::MathConstants<float>::pi * freq / sampleRate);
        k =static_cast<float>( 1.0f / Q);
        a1 = static_cast<float>(1.0f / (1.0f + g * (g + k)));
        a2 = g * a1;
        a3 = g * a2;
    }

    void lowpass(double sampleRate, double freq, double Q) noexcept
    {
        setCoefficients(sampleRate, freq, Q);
        m0 = 0.0f;
        m1 = 0.0f;
        m2 = 1.0f;
    }

    void highpass(double sampleRate, double freq, double Q) noexcept
    {
        setCoefficients(sampleRate, freq, Q);
        m0 = 1.0f;
        m1 = -k;
        m2 = -1.0f;
    }

    void bandpass(double sampleRate, double freq, double Q) noexcept
    {
        setCoefficients(sampleRate, freq, Q);
        m0 = 0.0f;
        m1 = k;     // paper says 1, but that is not same as RBJ bandpass
        m2 = 0.0f;
    }

    void notch(double sampleRate, double freq, double Q) noexcept
    {
        setCoefficients(sampleRate, freq, Q);
        m0 = 1.0f;
        m1 = -k;
        m2 = 0.0f;
    }

    void allpass(double sampleRate, double freq, double Q) noexcept
    {
        setCoefficients(sampleRate, freq, Q);
        m0 = 1.0f;
        m1 = -2.0f * k;
        m2 = 0.0f;
    }

    // Note: This is not the same as the RBJ peaking filter, since no dbGain.
    void peaking(double sampleRate, double freq, double Q) noexcept
    {
        setCoefficients(sampleRate, freq, Q);
        m0 = 1.0f;
        m1 = -k;
        m2 = -2.0f;
    }

    // Note: This is the same as the RBJ peaking EQ.
    void bell(double sampleRate, double freq, double Q, double dbGain) noexcept
    {
        const double A = std::pow(10.0, dbGain / 40.0);
        g = std::tan(juce::MathConstants<float>::pi * freq / sampleRate);
        k = 1.0 / (Q * A);
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f;
        m1 = k * (A * A - 1.0);
        m2 = 0.0f;
    }

    void lowShelf(double sampleRate, double freq, double Q, double dbGain) noexcept
    {
        const double A = std::pow(10.0, dbGain / 40.0);
        g = std::tan(juce::MathConstants<float>::pi * freq / sampleRate) / std::sqrt(A);
        k = 1.0 / Q;
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = 1.0f;
        m1 = k * (A - 1.0);
        m2 = (A * A - 1.0);
    }

    void highShelf(double sampleRate, double freq, double Q, double dbGain) noexcept
    {
        const double A = std::pow(10.0, dbGain / 40.0);
        g = std::tan(juce::MathConstants<float>::pi * freq / sampleRate) * std::sqrt(A);
        k = 1.0 / Q;
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        m0 = A * A;
        m1 = k * (1.0 - A) * A;
        m2 = (1.0 - A * A);
    }

    void reset() noexcept
    {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }

    float processSample(float v0) noexcept
    {
        float v3 = v0 - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;
        return m0 * v0 + m1 * v1 + m2 * v2;
    }

    void setSVF(const float type, const float sampleRate, const float frequency, const float resonance)
    {
	    switch(static_cast<int>(type))
	    {
			case 0:
                lowpass(sampleRate, frequency, resonance);
            break;
            case 1:
                highpass(sampleRate, frequency, resonance);
                break;
            case 2:
                bandpass(sampleRate, frequency, resonance);
                break;
	    default:
            lowpass(sampleRate, frequency, resonance);
            break;

	    }
    }

private:
    float g, k, a1, a2, a3;  // filter coefficients
    float m0, m1, m2;        // mix coefficients
    float ic1eq, ic2eq;      // internal state
};
