/*
  ==============================================================================

    SynthVoice.h
    Created: 8 Mar 2024 7:03:02pm
    Author:  blaze

  ==============================================================================
*/
#pragma once
#include "analogEG.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "SynthSound.h"
#include "FilterData.h"
#include "LFO.h"
#include "VAOsc.h"
#include "Osc.h"
#include "clipper.h"
#include "ModMatrix.h"
#include "SharedData.h"
class SynthVoice : public juce::SynthesiserVoice,juce::ValueTree::Listener
{
public:

	explicit SynthVoice::SynthVoice(juce::ValueTree& v):
	keyTrack(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000.0f,20.0f)), state(v),
	osc1{Osc(v),Osc(v)},osc2{VAOsc(v),VAOsc(v)},
	vaSVF(v),
	ladder(v),
	lfoGenerator{LFO(v),LFO(v)},
	modMatrix(v)
	{
		state.addListener(this);
	}
	SynthVoice::~SynthVoice() override= default;

	bool canPlaySound(juce::SynthesiserSound* sound) override
	{
		return dynamic_cast<SynthSound*>(sound) != nullptr;
	}

	void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
	               int currentPitchWheelPosition) override {
		const auto midiNote = midiNoteNumber;
		resetLFO();
		setOscillatorsFrequency(midiNote);
		setRandomPhase();
		ampEnv.noteOn();
		amp2Env.noteOn();
		modEnv.noteOn();
	}
	void setOscillatorsFrequency(const int midiNote)
	{
		for(auto i=0;i<2;++i)
		{
			osc1[i].setFrequency(frequency, midiNote);
			osc2[i].setFrequency(frequency, midiNote);
		}
	}
	void setRandomPhase()
	{
		phase = osc1[0].randomPhase();
		for(float & phase : phases)
		{
			phase = osc1[0].randomPhase();
		}
		for (auto i = 0; i < numChannelsToProcess; ++i)
		{
			osc1[i].setRandomPhase(phase,phases[0], phases[1], phases[2], phases[3], phases[4], phases[5]);
			osc2[i].setRandomPhase(phase);
		}
	}

	void stopNote(float velocity, bool allowTailOff) override
	{
		ampEnv.noteOff();
		amp2Env.noteOff();
		modEnv.noteOff();
		resetLFO();
		if (! allowTailOff || ! ampEnv.isActive() )
			clearCurrentNote();
	}
	void pitchWheelMoved(int newPitchWheelValue) override
	{
	}

	void controllerMoved(int controllerNumber, int newControllerValue) override
	{
	}

	void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels){

		reset();
		synthBuffer.setSize(2, samplesPerBlock, true, true, true);
		synthBuffer.clear();
		const juce::dsp::ProcessSpec spec{sampleRate,static_cast<uint32_t>(samplesPerBlock),static_cast<uint32_t>(outputChannels)};
		level.prepare(spec);
		keyTrack.prepare(spec);
		ladder.prepare(spec);
		level.setGainLinear(0.5f);
		for (auto i = 0; i < numChannelsToProcess; ++i)
		{
			lfoGenerator[i].prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
			osc1[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			osc2[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
		}
		modMatrix.setRouting(ModMatrix::modSource::kLFO,ModMatrix::modDestination::kFILTER_CUTOFF,1.0,&lfoMod,IDs::Cutoff);
		isPrepared = true;
	}
	void update()
	{
		setLFOParameters();
		getEnvelopeParameters();
		vaSVF.setParameters();
		ladder.setParameters();
		SVFEnabled = static_cast<int>(state[IDs::SVFEnabled]);
		lfoReset = state[IDs::LFOReset];
		osc1[0].setParameters();
		osc1[1].setParameters();
		osc2[0].setParameters();
		osc2[1].setParameters();
		panOSC1 = state[IDs::PanOsc1];
		panOSC2 = state[IDs::PanOsc2];
		updatePan();
	}
	void getEnvelopeParameters()
	{
		auto sampleRate = float(getSampleRate());
		float inverseSampleRate = 1.0f / sampleRate;

		ampEnv.attackMultiplier = std::exp(-inverseSampleRate *
			std::exp(5.5f - 0.075f * static_cast<float>(state[IDs::ADSR1Attack])));
		ampEnv.decayMultiplier = std::exp(-inverseSampleRate *
			std::exp(5.5f - 0.075f * static_cast<float>(state[IDs::ADSR1Decay])));
		ampEnv.sustainLevel = static_cast<float>(state[IDs::ADSR1Sustain]);
		float envRelease = state[IDs::ADSR1Release];
		if (envRelease < 1.0f) {
			ampEnv.releaseMultiplier = 0.75f;  // extra fast release
		} else {
			ampEnv.releaseMultiplier = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envRelease));
		}

		amp2Env.attackMultiplier =modEnv.attackMultiplier= std::exp(-inverseSampleRate *
			std::exp(5.5f - 0.075f * static_cast<float>(state[IDs::ADSR2Attack])));
		amp2Env.decayMultiplier=modEnv.decayMultiplier = std::exp(-inverseSampleRate *
			std::exp(5.5f - 0.075f * static_cast<float>(state[IDs::ADSR2Decay])));
		amp2Env.sustainLevel=modEnv.sustainLevel = static_cast<float>(state[IDs::ADSR2Sustain]);
		float env2Release = state[IDs::ADSR2Release];
		if (env2Release < 1.0f) {
			amp2Env.releaseMultiplier = modEnv.releaseMultiplier = 0.75f;  // extra fast release
		} else {
			amp2Env.releaseMultiplier=modEnv.releaseMultiplier =
				std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * env2Release));
		}

		reversedEnvelope = state[IDs::ReversedEnvelope];
		filterEnvelopeAmount = static_cast<float>(state[IDs::FilterEnvelopeAmount])*200.0f;
	}
	void updatePan()
	{
		panLeft1 = std::cos((juce::MathConstants<float>::halfPi/2.f)*(panOSC1+1));
		panLeft2 = std::cos((juce::MathConstants<float>::halfPi/2.f)*(panOSC2+1));
		panRight1 = std::sin((juce::MathConstants<float>::halfPi/2.f)*(panOSC1+1));
		panRight2 = std::sin((juce::MathConstants<float>::halfPi/2.f)*(panOSC2+1));
	}
	void setLFOParameters()
	{
		lfoGenerator[0].setParameters();
		lfoGenerator[1].setParameters();
	}
	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
		jassert(isPrepared);
		if (!isVoiceActive())
		{
			return;
		}

		setUpFirstOscBuffer(outputBuffer, numSamples);
		juce::dsp::AudioBlock<float> oscillatorSW{swBuffer};

		for(auto samples=0;samples<numSamples;++samples)
		{
			auto inputLeft = swBuffer.getReadPointer(0);
			auto inputRight = swBuffer.getReadPointer(1);
			auto outputLeft = swBuffer.getWritePointer(0);
			auto outputRight = swBuffer.getWritePointer(1);
			float channelLeft =0;
			float channelRight =0;

			envelopeMod = modEnv.nextValue()*filterEnvelopeAmount;

			cutOffMod = lfoGenerator[0].render(startSample,numSamples);
			auto nextAmpSample = ampEnv.nextValue();
			auto nextAmp2Sample = amp2Env.nextValue();


			channelLeft+=osc1[0].getNextSample()*nextAmpSample*panLeft1;
			channelLeft+=osc2[0].getNextSample()*nextAmp2Sample*panLeft2;
			channelRight+=osc1[1].getNextSample()*nextAmpSample*panRight1;
			channelRight+=osc2[1].getNextSample()*nextAmp2Sample*panRight2;

			if(SVFEnabled)
			{
				vaSVF.setCutOffMod(cutOffMod);
				channelLeft= vaSVF.processAudioSample(channelLeft,0);
				channelRight= vaSVF.processAudioSample(channelRight,1);
			}

			else
			{
				ladder.setCutOffMod(cutOffMod);
				channelLeft= ladder.processAudioSample(channelLeft,0);
				channelRight= ladder.processAudioSample(channelRight,1);
			}

			channelLeft = level.processSample(channelLeft);
			channelRight = level.processSample(channelRight);

			channelLeft = clip.process(channelLeft);
			channelRight = clip.process(channelRight);

			outputLeft[samples] = channelLeft+inputLeft[samples];
			outputRight[samples] = channelRight+inputRight[samples];
		}

		for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
		{
			outputBuffer.addFrom(channel, startSample, swBuffer, channel, 0, numSamples);
		}
		if (!ampEnv.isActive())
		{
			clearCurrentNote();
			resetOscillators();
		}

	}

	void setUpFirstOscBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
	{
		swBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		swBuffer.clear();
	}
	void calculateModAmount(const int& numSamples,const int& sample,const int& channel)
	{
		envelopeMod = modEnv.nextValue()*filterEnvelopeAmount;
		if(reversedEnvelope)
			cutOffMod = -envelopeMod + lfoGenerator[channel].render(sample,numSamples);
		else
			cutOffMod = envelopeMod + lfoGenerator[channel].render(sample,numSamples);
		cutOffMod = std::clamp(cutOffMod,20.0f,20000.0f);
	}

	void reset()
	{
		resetOscillators();
		vaSVF.reset(getSampleRate());
		resetLFO();
		level.reset();
		keyTrack.reset();
		ladder.reset();
		ampEnv.reset();
		amp2Env.reset();
		modEnv.reset();

	}
	void resetLFO()
	{
		if(lfoReset)
		{
			lfoGenerator[0].reset();
			lfoGenerator[1].reset();
		}
	}
	void resetOscillators()
	{
		for (auto& osc : osc1)
			osc.resetOsc();
		for (auto& osc : osc2)
			osc.resetOsc();
	}
	void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
	{

	}

	static constexpr int numChannelsToProcess{2};

private:
	float lfoMod;
	float panOSC1{0.0f};
	float panOSC2{0.0f};
	float panLeft1{0.f};
	float panLeft2{0.f};
	float panRight1{0.f};
	float panRight2{0.f};
	bool reversedEnvelope;
	float cutOffMod{0.0f};
	float filterEnvelopeAmount{0.0f};
	float envelopeMod{0.0f};
	float oldMod{0.0f};
	bool isPrepared{false};
	std::array<float, 6> phases{0.0f};
	float phase{ 0.0f };
	double frequency{};
	float type{};
	bool SVFEnabled;
	bool commonEnvelope;
	bool lfoReset;
	float oldFrequency{ 0.0f };
	std::array<Osc, numChannelsToProcess> osc1;
	std::array<VAOsc, numChannelsToProcess> osc2;
	ModMatrix modMatrix;
	juce::dsp::Gain<float> level;
	softClipper clip;
	ZVAFilter vaSVF;
	MOOGFilter ladder;
	std::array<LFO,2> lfoGenerator;
	juce::AudioBuffer<float> swBuffer;
	juce::AudioBuffer<float> synthBuffer;
	juce::AudioBuffer<float> vaBuffer;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
	juce::ValueTree state;
	analogEG ampEnv;
	analogEG amp2Env;
	analogEG modEnv;
	int updateRate{ 100 };
	int updateCounter{ updateRate };

};
