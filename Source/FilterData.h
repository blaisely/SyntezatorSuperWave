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
#include "helpers.h"


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
        float minCutoff = 20.0f; 
        float maxCutoff = 20000.0f; 
        float logMin = std::log10(minCutoff);
        float logMax = std::log10(maxCutoff);
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
            setFilterParam(tree[IDs::Cutoff], tree[IDs::Resonance], tree[IDs::FilterT]);
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
class ZVAFilter
{
public:
   ZVAFilter(juce::ValueTree& v):tree(v)
   {

   };
	~ZVAFilter()=default;

	bool reset (double sampleRate)
	{
		this->sampleRate = sampleRate;
		integrator_z[0] = 0.0;
		integrator_z[1] = 0.0;
		return true;
	}
	void setParameters()
	{
		if(vaFilterParameters.fc!=static_cast<float>(tree[IDs::Cutoff])||
			vaFilterParameters.Q!=static_cast<float>(tree[IDs::Resonance]))
		{
			vaFilterParameters.fc=tree[IDs::Cutoff];
			vaFilterParameters.Q=tree[IDs::Resonance];
			int filterType = static_cast<int>(tree[IDs::FilterT]);
			switch(filterType)
			{
				case 0:
				vaFilterParameters.filterAlgorithm = vaFilterAlgorithm::kSVF_LP;
				break;
			case 1:
				vaFilterParameters.filterAlgorithm = vaFilterAlgorithm::kSVF_HP;
				break;
			case 2:
				vaFilterParameters.filterAlgorithm = vaFilterAlgorithm::kSVF_BP;
				break;
			default:
				vaFilterParameters.filterAlgorithm = vaFilterAlgorithm::kSVF_LP;
				break;
			}
			calculateFilterCoeffs();
		}
	}
	double processAudioSample(double xn)
	{
		vaFilterAlgorithm filterAlgorithm = vaFilterParameters.filterAlgorithm;
		bool matchAnalogNyquistLPF = vaFilterParameters.matchAnalogNyquistLPF;
		if (vaFilterParameters.enableGainComp)
		{
			double peak_dB = dBPeakGainFor_Q(vaFilterParameters.Q);
			if (peak_dB > 0.0)
			{
				double halfPeak_dBGain = dB2Raw(-peak_dB / 2.0);
				xn *= halfPeak_dBGain;
			}
		}
		double hpf = alpha0*(xn - rho*integrator_z[0] - integrator_z[1]);

		// --- BPF Out
		double bpf = alpha*hpf + integrator_z[0];
		if (vaFilterParameters.enableNLP)
			bpf = softClipWaveShaper(bpf, 1.0);

		// --- LPF Out
		double lpf = alpha*bpf + integrator_z[1];

		// --- BSF Out
		double bsf = hpf + lpf;

		// --- finite gain at Nyquist; slight error at VHF
		double sn = integrator_z[0];

		// update memory
		integrator_z[0] = alpha*hpf + bpf;
		integrator_z[1] = alpha*bpf + lpf;

		double filterOutputGain = pow(10.0, vaFilterParameters.filterOutputGain_dB / 20.0);

		// return our selected type
		if (filterAlgorithm == vaFilterAlgorithm::kSVF_LP)
		{
			if (matchAnalogNyquistLPF)
				lpf += analogMatchSigma*(sn);
			return filterOutputGain*lpf;
		}
		else if (filterAlgorithm == vaFilterAlgorithm::kSVF_HP)
			return filterOutputGain*hpf;
		else if (filterAlgorithm == vaFilterAlgorithm::kSVF_BP)
			return filterOutputGain*bpf;
		else if (filterAlgorithm == vaFilterAlgorithm::kSVF_BS)
			return filterOutputGain*bsf;

		// --- unknown filter
		return filterOutputGain*lpf;
	}
	void calculateFilterCoeffs()
	{
		double fc = vaFilterParameters.fc;
		double Q = vaFilterParameters.Q;
		vaFilterAlgorithm filterAlgorithm = vaFilterParameters.filterAlgorithm;

		// --- normal Zavalishin SVF calculations here
		//     prewarp the cutoff- these are bilinear-transform filters
		double wd = kTwoPi*fc;
		double T = 1.0 / sampleRate;
		double wa = (2.0 / T)*tan(wd*T / 2.0);
		double g = wa*T / 2.0;


			// --- note R is the traditional analog damping factor zeta
			double R = vaFilterParameters.selfOscillate ? 0.0 : 1.0 / (2.0*Q);
			alpha0 = 1.0 / (1.0 + 2.0*R*g + g*g);
			alpha = g;
			rho = 2.0*R + g;

			// --- sigma for analog matching version
			double f_o = (sampleRate / 2.0) / fc;
			analogMatchSigma = 1.0 / (alpha*f_o*f_o);

	}
private:
	juce::ValueTree tree;
	enum class vaFilterAlgorithm {
		kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS
	};
	double sampleRate = 48000.0;
	double integrator_z[2];
	double alpha0=0.0;
	double alpha = 0.0;
	double rho =0.0;
	double beta = 0.0;
	double analogMatchSigma = 0.0;
	double kTwoPi = juce::MathConstants<double>::twoPi;
	struct ZVAFilterParameters
	{
		vaFilterAlgorithm filterAlgorithm = vaFilterAlgorithm::kSVF_LP;	///< va filter algorithm
		double fc = 1000.0;						///< va filter fc
		double Q = 0.707;						///< va filter Q
		double filterOutputGain_dB = 0.0;		///< va filter gain (normally unused)
		bool enableGainComp = true;			///< enable gain compensation (see book)
		bool matchAnalogNyquistLPF = true;		///< match analog gain at Nyquist
		bool selfOscillate = true;				///< enable selfOscillation
		bool enableNLP = true;
	};
	ZVAFilterParameters vaFilterParameters;

};
