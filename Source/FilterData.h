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
#include "SharedData.h"


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
		integrator_zLeft[0] = 0.0;
		integrator_zLeft[1] = 0.0;
		integrator_zRight[0] = 0.0;
		integrator_zRight[1] = 0.0;
		return true;
	}
	void setParameters()
	{
		if(vaFilterParameters.fc!=static_cast<float>(tree[IDs::Cutoff])||
			vaFilterParameters.Q!=static_cast<float>(tree[IDs::Resonance]) ||
			vaFilterParameters.filterDrive!=static_cast<double>(tree[IDs::FilterDrive]))
		{
			vaFilterParameters.fc=tree[IDs::Cutoff];
			vaFilterParameters.Q=tree[IDs::Resonance];
			vaFilterParameters.filterDrive = static_cast<double>(tree[IDs::FilterDrive]);
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
	void setCutOffMod(const float& modCutOff)
	{

		if(!juce::approximatelyEqual(modCutOff,static_cast<float>(vaFilterParameters.fc)))
		{
			vaFilterParameters.fc = modCutOff;
			calculateFilterCoeffs();
		}

	}
	double processAudioSample(double xn,int channel)
	{
		double* integrator_z = (channel == 0) ? integrator_zLeft : integrator_zRight;

		vaFilterAlgorithm filterAlgorithm = vaFilterParameters.filterAlgorithm;
		bool matchAnalogNyquistLPF = vaFilterParameters.matchAnalogNyquistLPF;
		double filterDrive = vaFilterParameters.filterDrive;

		filterDrive = juce::jmap(filterDrive,0.0,5.0);
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

		double bpf = alpha*hpf + integrator_z[0];
		if (vaFilterParameters.enableNLP)
			bpf = softClipWaveShaper(bpf, 1.0);

		double lpf = alpha*bpf + integrator_z[1];
		double bsf = hpf + lpf;
		double sn = integrator_z[0];
		integrator_z[0] = alpha*hpf + bpf;
		integrator_z[1] = alpha*bpf + lpf;

		double filterOutputGain = pow(10.0, vaFilterParameters.filterOutputGain_dB / 20.0);

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
	double integrator_zLeft[2];
	double integrator_zRight[2];
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
		double filterDrive =1.0;
	};
	ZVAFilterParameters vaFilterParameters;

};

class MOOGFilter: public juce::dsp::LadderFilter<float>
{
public:
	explicit MOOGFilter(juce::ValueTree& v):tree(v)
	{
	setEnabled(true);
	}
	~MOOGFilter()=default;
	void setParameters()
	{
		cutOffFrequency = tree[IDs::Cutoff];
		resonance = juce::jlimit(0.0f,2.0f,static_cast<float>(tree[IDs::Resonance]));
		driveAmount = tree[IDs::FilterDrive];
		setCutoffFrequencyHz(cutOffFrequency);
		setResonance(resonance);
		setDrive(driveAmount);
		type = static_cast<int>(tree[IDs::FilterT]);
		switch(type)
		{
		case 0:
			setMode(Mode::LPF24);
			break;
		case 1:
			setMode(Mode::HPF24);
			break;
		case 2:
			setMode(Mode::BPF24);
			break;
		default:
				setMode(Mode::LPF24);
			break;
		}
	}
	void setCutOffMod(const float& modCutOff)
	{
		if(!juce::approximatelyEqual(modCutOff,cutOffFrequency))
		setCutoffFrequencyHz(modCutOff);
	}
	float processAudioSample(const float& x,const int& channel)
	{
		updateSmoothers();
		return processSample(x,channel);
	}
private:
	float cutOffFrequency{};
	float resonance{};
	int type{};
	float driveAmount{};
	juce::ValueTree tree;
};
