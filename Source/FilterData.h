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
		filterResonance.reset(sampleRate,0.001f);
		filterCutOff.reset(sampleRate,0.001f);
		return true;
	}
	void setParameters(bool keyTrack, int keyTrackOffset, int currentMidiPitch)
	{
		if(vaFilterParameters.fc!=static_cast<float>(tree[IDs::Cutoff])||
			vaFilterParameters.Q!=static_cast<float>(tree[IDs::Resonance]) ||
			vaFilterParameters.filterDrive!=static_cast<double>(tree[IDs::FilterDrive])||keyTrack)
		{
			if(keyTrack)
			{
				float fc  = juce::MidiMessage::getMidiNoteInHertz(currentMidiPitch);
				fc*=std::pow(2.0f,keyTrackOffset/12);
				vaFilterParameters.fc = fc;
			}
			else
				vaFilterParameters.fc=tree[IDs::Cutoff];

			vaFilterParameters.selfOscillate = false;
			vaFilterParameters.Q=static_cast<float>(tree[IDs::Resonance])/10.f;

			if(vaFilterParameters.Q<0.1f)
				vaFilterParameters.Q = 0.1f;
			if(vaFilterParameters.Q==10.f)
				vaFilterParameters.selfOscillate = true;

			vaFilterParameters.filterDrive =juce::jmap(static_cast<float>(tree[IDs::FilterDrive]),1.f,2.f);

			int filterType = (tree[IDs::FilterT]);
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
	double processAudioSample(double xn,int channel)
	{
		double* integrator_z = (channel == 0) ? integrator_zLeft : integrator_zRight;

		vaFilterAlgorithm filterAlgorithm = vaFilterParameters.filterAlgorithm;
		bool matchAnalogNyquistLPF = vaFilterParameters.matchAnalogNyquistLPF;
		double filterDrive = vaFilterParameters.filterDrive;

		if (vaFilterParameters.enableGainComp)
		{
			double peak_dB = dBPeakGainFor_Q(vaFilterParameters.Q);
			if (peak_dB > 0.0)
			{
				double halfPeak_dBGain = dB2Raw(-peak_dB / 2.0);
				xn *= halfPeak_dBGain;
			}
		}
		xn = arctangentSaturation(xn,filterDrive);
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
		float currentCutoff = vaFilterParameters.fc;
		float currentResonance = vaFilterParameters.Q;
		float modulatedCutoff;
		float modulatedResonance = modValue[kRESONANCE] * 10.0f;
		modulatedResonance = currentResonance + modulatedResonance;
		if(modulatedResonance==10.f)
			vaFilterParameters.selfOscillate = true;
		filterResonance.setTargetValue(juce::jlimit(0.1f, 10.0f, modulatedResonance));
		if (!juce::approximatelyEqual(modValue[kCUTOFF],0.00f))
		{
			modulatedCutoff = modValue[kCUTOFF] * 2.5f;
			modulatedCutoff = currentCutoff * std::exp(modulatedCutoff);
			modulatedCutoff = juce::jlimit(20.0f, 20480.0f, modulatedCutoff);
		}
		else
			modulatedCutoff = currentCutoff;

		filterCutOff.setTargetValue(modulatedCutoff);
		double fc = filterCutOff.getNextValue();
		double Q = filterResonance.getNextValue();
		vaFilterAlgorithm filterAlgorithm = vaFilterParameters.filterAlgorithm;

		// Filter coefficient calculations remain the same
		double wd = kTwoPi * fc;
		double T = 1.0 / sampleRate;
		double wa = (2.0 / T) * tan(wd * T / 2.0);
		double g = wa * T / 2.0;

		double R = vaFilterParameters.selfOscillate ? 0.0 : 1.0 / (2.0 * Q);
		alpha0 = 1.0 / (1.0 + 2.0 * R * g + g * g);
		alpha = g;
		rho = 2.0 * R + g;

		double f_o = (sampleRate / 2.0) / fc;
		analogMatchSigma = 1.0 / (alpha * f_o * f_o);
	}

	float* getModCutOff()
	{
		return &modValue[kCUTOFF];
	}
	float* getModResonance()
	{
		return &modValue[kRESONANCE];
	}
	void setModResonance(const float res)
	{
		modValue[kRESONANCE] = res;
	}


private:
	juce::ValueTree tree;
	enum class vaFilterAlgorithm {
		kSVF_LP, kSVF_HP, kSVF_BP, kSVF_BS
	};
	enum{kCUTOFF,kRESONANCE,kNumDest};
	std::array<float,kNumDest> modValue{0.0f};
	double sampleRate = 48000.0;
	double integrator_zLeft[2];
	double integrator_zRight[2];
	double alpha0=0.0;
	double alpha = 0.0;
	double rho =0.0;
	double beta = 0.0;
	double analogMatchSigma = 0.0;
	double kTwoPi = juce::MathConstants<double>::twoPi;
	juce::SmoothedValue<float> filterResonance {0.0f};
	juce::SmoothedValue<float> filterCutOff{0.0f};


	struct ZVAFilterParameters
	{
		vaFilterAlgorithm filterAlgorithm = vaFilterAlgorithm::kSVF_LP;	///< va filter algorithm
		double fc = 1000.0;						///< va filter fc
		double Q = 0.707;						///< va filter Q
		double filterOutputGain_dB = 5.0;		///< va filter gain (normally unused)
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
	void setParameters(bool keyTrack, int keyTrackOffset, int currentMidiPitch)
	{
		cutOffFrequency = tree[IDs::Cutoff];
		resonance = juce::jmap(static_cast<float>(tree[IDs::Resonance])/10.0f,0.0f,10.0f,0.0f,1.0f);
		if(resonance<=0)
				resonance=0.1f;
		driveAmount = juce::jmap(static_cast<float>(tree[IDs::FilterDrive]),1.f,100.f,1.f,10.f);
		if(keyTrack)
		{
			cutOffFrequency  = juce::MidiMessage::getMidiNoteInHertz(currentMidiPitch);
			cutOffFrequency*=std::pow(2.0f,keyTrackOffset/12);
			setCutoffFrequencyHz(cutOffFrequency);
		}
		else if(!keyTrack)
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
			setMode(Mode::BPF24);
			break;
		case 2:
			setMode(Mode::HPF24);
			break;
		default:
			setMode(Mode::LPF24);
			break;
		}
	}
	float processAudioSample(const float& x,const int& channel)
	{
		return processSample(x,channel);
	}
	float* getModCutOff()
	{
		return &modValue[kCUTOFF];
	}
	float* getModResonance()
	{
		return &modValue[kRESONANCE];
	}
	void setModResonance(const float res)
	{
		modValue[kRESONANCE] = res;
	}
	void setModCutOff(const float fc)
	{
		modValue[kCUTOFF] = fc;
	}
	void updateModulation()
	{
		if(!juce::approximatelyEqual(modValue[kCUTOFF],0.00f))
		{
			float targetModulatedCutOff = modValue[kCUTOFF]*2.5f;
			targetModulatedCutOff = cutOffFrequency * std::exp(targetModulatedCutOff);
			targetModulatedCutOff = juce::jlimit(20.0f, 20480.0f, targetModulatedCutOff);
			setCutoffFrequencyHz(targetModulatedCutOff);
		}
		float modRes = modValue[kRESONANCE];
		modRes = std::clamp(resonance+modRes,0.f,1.f);
		setResonance(modRes);

	}
	void updateSmoothing()
	{
		updateSmoothers();
	}

private:
	enum{kCUTOFF,kRESONANCE,kNumDest};
	std::array<float,kNumDest> modValue {0.0f};
	float cutOffFrequency{};
	float resonance{};
	int type{};
	float driveAmount{};

	juce::ValueTree tree;
};
