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
	TPTFilter{Filter(v),Filter(v)},
	vaSVF(v),
	ladder(v),
	lfoGenerator(v)
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
			osc1[i].setFrequency(frequencyFirstOsc, midiNote);
			osc2[i].setFrequency(frequencyFirstOsc, midiNote);
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
		setOscillatorsFrequency(midiNote);
		setRandomPhase();
		envelope.noteOn();
	}

	void stopNote(float velocity, bool allowTailOff) override
	{
		envelope.noteOff();
		allowTailOff = true;
		if (envelope.isActive() == false)
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
		synthesisBuffer.setSize(2, samplesPerBlock, true, true, true);
		synthesisBuffer.clear();
		juce::dsp::ProcessSpec spec{};
		spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = outputChannels;
		envelope.setSampleRate(getSampleRate());
		level.prepare(spec);
		keyTrack.prepare(spec);
		ladder.prepare(spec);
		level.setGainLinear(1.0f);
		lfoGenerator.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
		for (auto i = 0; i < numChannelsToProcess; ++i)
		{
			osc1[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			osc2[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			TPTFilter[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
		}
		isPrepared = true;
	}

	void processSoftClip(int numSamples)
	{
		for(auto channel =0; channel<numChannelsToProcess; ++channel)
		{
			auto input = synthBuffer1.getReadPointer(channel);
			auto output = synthBuffer1.getWritePointer(channel);

			for(auto sample =0;sample<numSamples;++sample)
				output[sample]=clip.process(input[sample]);
		}
	}

	void processFilter(int numSamples)
	{
		SVFEnabled = static_cast<int>(state[IDs::SVFEnabled]);
		if(SVFEnabled)
		{
			vaSVF.setParameters();
			auto inputLeft = synthBuffer1.getReadPointer(0);
			auto outputLeft = synthBuffer1.getWritePointer(0);
			auto outputRight = synthBuffer1.getWritePointer(1);
			for(auto sample =0;sample<numSamples;++sample)
			{
				auto y = vaSVF.processAudioSample(inputLeft[sample]);
				outputLeft[sample] = outputRight[sample] = y;
			}
		}
		else
		{
			ladder.setParameters();
			for(auto channel=0;channel<numChannelsToProcess;++channel)
			{
				auto input = synthBuffer1.getReadPointer(channel);
				auto output = synthBuffer1.getWritePointer(channel);
				for(auto sample=0;sample<numSamples;++sample)
				{
					output[sample] = ladder.processAudioSample(input[sample],channel);
				}
			}
		}
	}

	void processLFO(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
	{
		lfoGenerator.setParameters();
		float lfoMod = lfoGenerator.render(outputBuffer,startSample,numSamples);
		float currentCutoff = state[IDs::Cutoff];
		filterZip += 0.005f * (lfoMod - filterZip);
		float modulatedCutoff = currentCutoff * filterZip;
		modulatedCutoff+=currentCutoff;
		modulatedCutoff = std::clamp(modulatedCutoff, 20.0f, 20000.0f);
		state.setProperty(IDs::Cutoff,modulatedCutoff,nullptr);
	}

	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
		jassert(isPrepared);
		if (!isVoiceActive())
		{
			return;
		}
		envelope.setParameters(envelopeParameters);
		setUpFirstOscBuffer(outputBuffer, numSamples);
		setUpSecondOscBuffer(outputBuffer, numSamples);
		juce::dsp::AudioBlock<float> oscillatorSW{synthBuffer1};
		juce::dsp::AudioBlock<float> oscillatorVA{synthBuffer2};

		for (auto i = 0; i < numChannelsToProcess; i++)
		{
			osc1[i].getNextBlock(oscillatorSW, i);
			osc2[i].getNextBlock(oscillatorVA, i);
		}
		oscillatorSW.add(oscillatorVA);

		processLFO(outputBuffer, startSample, numSamples);

		processFilter(numSamples);

		const auto context = juce::dsp::ProcessContextReplacing<float>(oscillatorSW);
		level.process(context);
		envelope.applyEnvelopeToBuffer(synthBuffer1, 0, numSamples);

		processSoftClip(numSamples);

		for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
		{
			outputBuffer.addFrom(channel, startSample, synthBuffer1, channel, 0, numSamples);
		}
		if (!envelope.isActive())
			clearCurrentNote();
	}

	void setUpFirstOscBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
	{
		synthBuffer1.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		synthBuffer1.clear();
	}

	void setUpSecondOscBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
	{
		synthBuffer2.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		synthBuffer2.clear();
	}
	void reset()
	{
		for (int i = 0; i < numChannelsToProcess; i++) {
			osc1[i].resetOsc();
			osc2[i].resetOsc();
			TPTFilter[i].resetAll();
			vaSVF.reset(getSampleRate());
		}
		lfoGenerator.reset();
		level.reset();
		envelope.reset();
		keyTrack.reset();
		ladder.reset();
	}
	void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
	{

	}

	void setEnvelope(float* attack, float* decay, float* sustain, float* release)
	{
		envelopeParameters.attack = *attack;
		envelopeParameters.decay = *decay;
		envelopeParameters.sustain = *sustain;
		envelopeParameters.release = *release;
	}

	static constexpr int numChannelsToProcess{2};

private:
	bool isPrepared{false};
	std::array<float, 6> phases{0.0f};
	float phase{ 0.0f };
	float t = 0;
	double frequencyFirstOsc{};
	double frequencySecondOsc{ 0.0f };
	float type{};
	float filterZip{0.0f};
	bool SVFEnabled;
	float oldFrequency{ 0.0f };
	std::array<Osc, numChannelsToProcess> osc1;
	std::array<VAOsc, numChannelsToProcess> osc2;
	juce::dsp::Gain<float> level;
	softClipper clip;
	std::array<Filter,numChannelsToProcess> TPTFilter;
	ZVAFilter vaSVF;
	MOOGFilter ladder;
	LFO lfoGenerator;
	juce::AudioBuffer<float> synthBuffer1;
	juce::AudioBuffer<float> synthesisBuffer;
	juce::AudioBuffer<float> synthBuffer2;
	juce::ADSR envelope;
	juce::ADSR::Parameters envelopeParameters;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
	juce::ValueTree state;
};
