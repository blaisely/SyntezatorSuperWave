/*
  ==============================================================================

    SynthVoice.h
    Created: 8 Mar 2024 7:03:02pm
    Author:  blaze

  ==============================================================================
*/
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "SynthSound.h"
#include "FilterData.h"
#include "LFO.h"
#include "VAOsc.h"
#include "Osc.h"
#include "clipper.h"
#include "SharedData.h"
class SynthVoice : public juce::SynthesiserVoice,juce::ValueTree::Listener
{
public:

	explicit SynthVoice::SynthVoice(juce::ValueTree& v):
	keyTrack(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000.0f,20.0f)), state(v),
	osc1{Osc(v),Osc(v)},osc2{VAOsc(v),VAOsc(v)},
	vaSVF(v),
	ladder(v),
	lfoGenerator{LFO(v),LFO(v)}
	{
		state.addListener(this);
	}
	SynthVoice::~SynthVoice() override= default;

	bool canPlaySound(juce::SynthesiserSound* sound) override
	{
		return dynamic_cast<SynthSound*>(sound) != nullptr;
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

	void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
	               int currentPitchWheelPosition) override {
		const auto midiNote = midiNoteNumber;
		resetLFO();
		setOscillatorsFrequency(midiNote);
		setRandomPhase();
		ampEnvelope.noteOn();
		amp2Envelope.noteOn();
		modEnvelope.noteOn();
	}

	void resetLFO()
	{
		if(lfoReset)
		{
			lfoGenerator[0].reset();
			lfoGenerator[1].reset();
		}
	}

	void stopNote(float velocity, bool allowTailOff) override
	{
		ampEnvelope.noteOff();
		amp2Envelope.noteOff();
		modEnvelope.noteOff();
		allowTailOff = false;
		resetLFO();
		if (ampEnvelope.isActive() == false)
		{
			clearCurrentNote();
			osc1[0].resetOsc();
			osc2[1].resetOsc();
			reset();
		}
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
		ampEnvelope.setSampleRate(getSampleRate());
		amp2Envelope.setSampleRate(getSampleRate());
		modEnvelope.setSampleRate(getSampleRate());
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
		isPrepared = true;
	}
	void update()
	{
		setLFOParameters();
		getEnvelopeParameters();
		setEnvelopeParameters();
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

			auto nextAmpSample = ampEnvelope.getNextSample();
			auto nextAmp2Sample = amp2Envelope.getNextSample();

			channelLeft+=osc1[0].getNextSample()*nextAmpSample*panLeft1;
			channelLeft+=osc2[0].getNextSample()*nextAmp2Sample*panLeft2;
			channelRight+=osc1[1].getNextSample()*nextAmpSample*panRight1;
			channelRight+=osc2[1].getNextSample()*nextAmp2Sample*panRight2;


			if(SVFEnabled)
			{
				channelLeft= vaSVF.processAudioSample(channelLeft,0);
				channelRight= vaSVF.processAudioSample(channelRight,1);
			}

			else
			{
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
		if (!ampEnvelope.isActive())
			clearCurrentNote();
	}

	void setUpFirstOscBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
	{
		swBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		swBuffer.clear();
	}

	void getEnvelopeParameters()
	{
		ampEnvelopeParameters.attack = state[IDs::ADSR1Attack];
		ampEnvelopeParameters.decay = state[IDs::ADSR1Decay];
		ampEnvelopeParameters.sustain = state[IDs::ADSR1Sustain];
		ampEnvelopeParameters.release = state[IDs::ADSR1Release];

		amp2EnvelopeParameters.attack = modEnvelopeParameters.attack = state[IDs::ADSR2Attack];
		amp2EnvelopeParameters.decay = modEnvelopeParameters.decay= state[IDs::ADSR2Decay];
		amp2EnvelopeParameters.sustain = modEnvelopeParameters.sustain = state[IDs::ADSR2Sustain];
		amp2EnvelopeParameters.release = modEnvelopeParameters.release = state[IDs::ADSR2Release];
		reversedEnvelope = state[IDs::ReversedEnvelope];
		filterEnvelopeAmount = static_cast<float>(state[IDs::FilterEnvelopeAmount])*200.0f;
	}
	void setEnvelopeParameters()
	{
		ampEnvelope.setParameters(ampEnvelopeParameters);
		amp2Envelope.setParameters(amp2EnvelopeParameters);
		modEnvelope.setParameters(modEnvelopeParameters);
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

	void calculateModAmount(const int& numSamples,const int& sample,const int& channel)
	{
		envelopeMod = modEnvelope.getNextSample()*filterEnvelopeAmount;
		if(reversedEnvelope)
			cutOffMod = -envelopeMod + lfoGenerator[channel].render(sample,numSamples);
		else
			cutOffMod = envelopeMod + lfoGenerator[channel].render(sample,numSamples);
		cutOffMod = std::clamp(cutOffMod,20.0f,20000.0f);
	}

	void reset()
	{
		for (int i = 0; i < numChannelsToProcess; i++) {
			osc1[i].resetOsc();
			osc2[i].resetOsc();
			vaSVF.reset(getSampleRate());
		}
		if(lfoReset)
		{
			lfoGenerator[0].reset();
			lfoGenerator[1].reset();
		}
		level.reset();
		ampEnvelope.reset();
		amp2Envelope.reset();
		modEnvelope.reset();
		keyTrack.reset();
		ladder.reset();
	}
	void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
	{

	}

	static constexpr int numChannelsToProcess{2};

private:
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
	int updateRate{100};
	int updateCounter{updateRate};
	std::array<Osc, numChannelsToProcess> osc1;
	std::array<VAOsc, numChannelsToProcess> osc2;
	juce::dsp::Gain<float> level;
	softClipper clip;
	ZVAFilter vaSVF;
	MOOGFilter ladder;
	std::array<LFO,2> lfoGenerator;
	juce::AudioBuffer<float> swBuffer;
	juce::AudioBuffer<float> synthBuffer;
	juce::AudioBuffer<float> vaBuffer;
	juce::ADSR ampEnvelope;
	juce::ADSR amp2Envelope;
	juce::ADSR modEnvelope;
	juce::ADSR::Parameters ampEnvelopeParameters;
	juce::ADSR::Parameters amp2EnvelopeParameters;
	juce::ADSR::Parameters modEnvelopeParameters;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
	juce::ValueTree state;
};
