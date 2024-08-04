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
#include "SharedData.h"
class SynthVoice : public juce::SynthesiserVoice,juce::ValueTree::Listener
{
public:
	
	SynthVoice::SynthVoice(juce::ValueTree& v):
	keyTrack(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000.0f,20.0f)), state(v),
	osc1{Osc(v),Osc(v)},osc2{VAOsc(v),VAOsc(v)},TPTFilter{Filter(v),Filter(v)}
	{
		state.addListener(this);
	}
	SynthVoice::~SynthVoice(){}
	void setEnvelope(float* attack, float* decay, float* sustain, float* release)
	{
		envelopeParameters.attack = *attack;
		envelopeParameters.decay = *decay;
		envelopeParameters.sustain = *sustain;
		envelopeParameters.release = *release;
	}
	bool canPlaySound(juce::SynthesiserSound* sound) override
	{
		return dynamic_cast<SynthSound*>(sound) != nullptr;
	}

	void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
	               int currentPitchWheelPosition) override {
		frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
		frequency2 = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

		osc1[0].getPhaseIncrement(frequency, midiNoteNumber);
		osc1[1].getPhaseIncrement(frequency, midiNoteNumber);
		//state.setPropertyExcludingListener(this,IDs::MidiNote,midiNoteNumber,nullptr);//setting global midi note
		//state.setProperty(IDs::SWFrequency, frequency, nullptr);//setting frequency for SW osc
		//state.setProperty(IDs::VAFrequency, frequency2, nullptr);//setting frequency for VA osc
		oldFrequency = frequency;
		phase = osc1[0].randomPhase();
		for(auto i=0;i<phases.size();i++)
		{
			phases[i] = osc1[0].randomPhase();
		}
		for (auto i = 0; i < numChannelsToProcess; ++i)
		{
			osc1[i].setRandomPhase(phase,phases[0], phases[1], phases[2], phases[3], phases[4], phases[5]);
			osc2[i].setRandomPhase(phase);
		}
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
			phase = 0.0f;
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
		juce::dsp::ProcessSpec spec;
		spec.maximumBlockSize = samplesPerBlock;
		spec.sampleRate = sampleRate;
		spec.numChannels = outputChannels;
		envelope.setSampleRate(getSampleRate());
		level.prepare(spec);
		keyTrack.prepare(spec);
		level.setGainLinear(1.0f);
		for (auto i = 0; i < numChannelsToProcess; i++)
		{
			osc1[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			osc2[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			TPTFilter[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			Lfo1[i].prepareToPlay(sampleRate, outputChannels, samplesPerBlock);
			Lfo2[i].prepareToPlay(sampleRate, outputChannels, samplesPerBlock);
		}
		isPrepared = true;
	}
	void checkForFilter(float *on)
	{
		filterOn = *on;

	}
	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
		jassert(isPrepared);
		if (!isVoiceActive())
		{
			return;
		}
		envelope.setParameters(envelopeParameters);
		synthBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		synthBuffer.clear();
		synthBuffer_osc2.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		synthBuffer_osc2.clear();
		juce::dsp::AudioBlock<float> audioBlock{synthBuffer};
		juce::dsp::AudioBlock<float> audioBlock_osc2{synthBuffer_osc2};

			for (auto i = 0; i < numChannelsToProcess; i++)
			{
				osc1[i].getNextBlock(audioBlock, i);
				osc2[i].getNextBlock(audioBlock_osc2, i);
			}
			audioBlock.add(audioBlock_osc2);

		
			for (auto i = 0; i < numChannelsToProcess; i++)
			{
				Lfo1[i].renderNextBlock(synthBuffer,startSample,numSamples);
				TPTFilter[i].setClampedCutOff(Lfo1[i].getModValueLfo1());
				TPTFilter[i].processNextBlock(synthBuffer, 0, numSamples);
			}
		
		/*
		else if (filterOn==1)
		{
			for (auto channel = 0; channel < numChannelsToProcess; channel++)
			{
				auto audioIn = synthBuffer.getReadPointer(channel);
				auto audioOut = synthBuffer.getWritePointer(channel);
				for (auto k = 0; k < numSamples; k++)
				{
					audioOut[k] = SVF[channel].processSample(audioIn[k]);
				}
			}
		}*/
		const auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);
		level.process(context);
		envelope.applyEnvelopeToBuffer(synthBuffer, 0, numSamples);
		for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
		{
			outputBuffer.addFrom(channel, startSample, synthBuffer, channel, 0, numSamples);
		}
		if (!envelope.isActive())
			clearCurrentNote();
	}
	void reset()
	{
		for (int i = 0; i < numChannelsToProcess; i++) {
			osc1[i].resetOsc();
			osc2[i].resetOsc();
			TPTFilter[i].resetAll();
			Lfo1[i].reset();
			SVF[i].reset();
		}
		level.reset();
		envelope.reset();
		keyTrack.reset();
		
	}
	void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
	{
		
	}

	float frequency;
	float type;
	static constexpr int numChannelsToProcess{2};

private:
	
	juce::dsp::Gain<float> level;
	bool isPrepared{false};
	std::array<Filter,numChannelsToProcess> TPTFilter;
	std::array<StateVariableFilter, numChannelsToProcess> SVF;
	std::array<LFO, numChannelsToProcess> Lfo1;
	std::array<LFO, numChannelsToProcess> Lfo2;
	juce::AudioBuffer<float> synthBuffer;
	juce::AudioBuffer<float> synthesisBuffer;
	juce::AudioBuffer<float> synthBuffer_osc2;
	std::array<float, 6> phases{0.0f};
	float phase{ 0.0f };
	float t = 0;
	float frequency2{ 0.0f };
	std::array<Osc, numChannelsToProcess> osc1;
	std::array<VAOsc, numChannelsToProcess> osc2;
	juce::ADSR envelope;
	juce::ADSR::Parameters envelopeParameters;
	int filterOn;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
	float oldFrequency{ 0.0f };
	enum class modSource
	{
		LFO1,
		LFO2,
		ADSR1,
		ADSR2
	};
	enum class modDestination
	{
		CutOff,
		Resonance,
		OSC1Vol,
		OSC2Vol,
		SuperDetune,
		SuperVol,
	};
	struct modulator
	{
		modSource* src;
		modDestination* dest;
		float modDepth;
	};

	juce::ValueTree state;
	
	
};
