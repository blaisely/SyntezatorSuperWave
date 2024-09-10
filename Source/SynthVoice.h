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
		allowTailOff = true;
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
		level.setGainLinear(1.0f);

		for (auto i = 0; i < numChannelsToProcess; ++i)
		{
			lfoGenerator[i].prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
			osc1[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			osc2[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
		}
		isPrepared = true;
	}
	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
		jassert(isPrepared);
		if (!isVoiceActive())
		{
			return;
		}
		lfoReset = state[IDs::LFOReset];

		getEnvelopeParameters();
		setEnvelopeParameters();
		setUpFirstOscBuffer(outputBuffer, numSamples);
		setUpSecondOscBuffer(outputBuffer, numSamples);
		juce::dsp::AudioBlock<float> oscillatorSW{swBuffer};
		juce::dsp::AudioBlock<float> oscillatorVA{vaBuffer};

		processOsc(oscillatorSW, oscillatorVA);

		processEnvelope(numSamples, oscillatorSW, oscillatorVA);

		setLFOParameters();

		processFilter(numSamples,startSample);

		processGain(oscillatorSW);

		processSoftClip(numSamples);

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

	void setUpSecondOscBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
	{
		vaBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		vaBuffer.clear();
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

	void setLFOParameters()
	{
		lfoGenerator[0].setParameters();
		lfoGenerator[1].setParameters();
	}

	void processGain(juce::dsp::AudioBlock<float> oscillatorSW)
	{
		const auto context = juce::dsp::ProcessContextReplacing<float>(oscillatorSW);
		level.process(context);
	}
	void processSoftClip(const int& numSamples)
	{
		for(auto channel =0; channel<numChannelsToProcess; ++channel)
		{
			auto input = swBuffer.getReadPointer(channel);
			auto output = swBuffer.getWritePointer(channel);

			for(auto sample =0;sample<numSamples;++sample)
				output[sample]=clip.process(input[sample]);
		}
	}



	void processFilter(const int& numSamples,const int& startSample)
	{
		SVFEnabled = static_cast<int>(state[IDs::SVFEnabled]);
		if(SVFEnabled)
		{
			vaSVF.setParameters();
			auto inputLeft = swBuffer.getReadPointer(0);
			auto outputLeft = swBuffer.getWritePointer(0);
			auto outputRight = swBuffer.getWritePointer(1);
			for(auto sample =0;sample<numSamples;++sample)
			{
				calculateModAmount(numSamples, sample,0);
				vaSVF.setCutOffMod(cutOffMod);
				float y = vaSVF.processAudioSample(inputLeft[sample]);
				outputLeft[sample] = outputRight[sample] = y;
			}
		}
		else
		{
			ladder.setParameters();
			for(auto channel=0;channel<numChannelsToProcess;++channel)
			{
				auto input = swBuffer.getReadPointer(channel);
				auto output = swBuffer.getWritePointer(channel);
				for(auto sample=0;sample<numSamples;++sample)
				{
					calculateModAmount(numSamples,sample,channel);
					ladder.setCutOffMod(cutOffMod);
					output[sample] = ladder.processAudioSample(input[sample],channel);
				}
			}
		}
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

	void processOsc(juce::dsp::AudioBlock<float>& oscillatorSW, juce::dsp::AudioBlock<float>& oscillatorVA)
	{
		for (auto i = 0; i < numChannelsToProcess; i++)
		{
			osc1[i].getNextBlock(oscillatorSW, i);
			osc2[i].getNextBlock(oscillatorVA, i);
		}
	}

	void processEnvelope(const int& numSamples, juce::dsp::AudioBlock<float>& oscillatorSW,
		juce::dsp::AudioBlock<float>& oscillatorVA)
	{
		commonEnvelope = state[IDs::CommonEnvelope];
		if(commonEnvelope)
		{
			oscillatorSW.add(oscillatorVA);
			ampEnvelope.applyEnvelopeToBuffer(swBuffer, 0, numSamples);
		}
		else
		{
			ampEnvelope.applyEnvelopeToBuffer(swBuffer, 0, numSamples);
			amp2Envelope.applyEnvelopeToBuffer(vaBuffer,0,numSamples);
			oscillatorSW.add(oscillatorVA);
		}
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
