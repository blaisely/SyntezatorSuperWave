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
		setOscillatorsFrequency(midiNote);
		setRandomPhase();
		ampEnvelope.noteOn();
		modEnvelope.noteOn();
		if(lfoReset)
			lfoGenerator.reset();
	}

	void stopNote(float velocity, bool allowTailOff) override
	{
		ampEnvelope.noteOff();
		modEnvelope.noteOff();
		allowTailOff = true;
		if (ampEnvelope.isActive() == false)
		{
			clearCurrentNote();
			osc1[0].resetOsc();
			osc2[1].resetOsc();
			reset();
			if(lfoReset)
				lfoGenerator.reset();
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
		modEnvelope.setSampleRate(getSampleRate());
		level.prepare(spec);
		keyTrack.prepare(spec);
		ladder.prepare(spec);
		level.setGainLinear(1.0f);
		lfoGenerator.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
		for (auto i = 0; i < numChannelsToProcess; ++i)
		{
			osc1[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
			osc2[i].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
		}
		isPrepared = true;
	}

	void processSoftClip(int numSamples)
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
					output[sample] = ladder.processAudioSample(input[sample],channel);
				}
			}
		}
	}

	void processLFO( int startSample,const int& numSamples)
	{

		float lfoMod = lfoGenerator.render(startSample,numSamples);

			float currentCutoff = state[IDs::Cutoff];
			filterZip += 0.005f * (lfoMod - filterZip);
			float modulatedCutoff = currentCutoff * filterZip;
			modulatedCutoff+=currentCutoff;
			modulatedCutoff = std::clamp(modulatedCutoff, 20.0f, 20000.0f);
			state.setProperty(IDs::Cutoff,modulatedCutoff,nullptr);

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
			modEnvelope.applyEnvelopeToBuffer(vaBuffer,0,numSamples);
			oscillatorSW.add(oscillatorVA);
		}
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
		ampEnvelope.setParameters(ampEnvelopeParameters);
		modEnvelope.setParameters(modEnvelopeParameters);
		setUpFirstOscBuffer(outputBuffer, numSamples);
		setUpSecondOscBuffer(outputBuffer, numSamples);
		juce::dsp::AudioBlock<float> oscillatorSW{swBuffer};
		juce::dsp::AudioBlock<float> oscillatorVA{vaBuffer};

		processOsc(oscillatorSW, oscillatorVA);

		processEnvelope(numSamples, oscillatorSW, oscillatorVA);

		lfoGenerator.setParameters();
		processLFO(startSample, numSamples);


		processFilter(numSamples,startSample);

		const auto context = juce::dsp::ProcessContextReplacing<float>(oscillatorSW);
		level.process(context);

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
	void reset()
	{
		for (int i = 0; i < numChannelsToProcess; i++) {
			osc1[i].resetOsc();
			osc2[i].resetOsc();
			vaSVF.reset(getSampleRate());
		}
		lfoGenerator.reset();
		level.reset();
		ampEnvelope.reset();
		modEnvelope.reset();
		keyTrack.reset();
		ladder.reset();
	}
	void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
	{

	}

	void getEnvelopeParameters()
	{
		ampEnvelopeParameters.attack = state[IDs::ADSR1Attack];
		ampEnvelopeParameters.decay = state[IDs::ADSR1Decay];
		ampEnvelopeParameters.sustain = state[IDs::ADSR1Sustain];
		ampEnvelopeParameters.release = state[IDs::ADSR1Release];

		modEnvelopeParameters.attack = state[IDs::ADSR2Attack];
		modEnvelopeParameters.decay = state[IDs::ADSR2Decay];
		modEnvelopeParameters.sustain = state[IDs::ADSR2Sustain];
		modEnvelopeParameters.release = state[IDs::ADSR2Release];
	}

	static constexpr int numChannelsToProcess{2};

private:
	float oldMod{0.0f};
	bool isPrepared{false};
	std::array<float, 6> phases{0.0f};
	float phase{ 0.0f };
	double frequency{};
	float type{};
	float filterZip{0.0f};
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
	LFO lfoGenerator;
	juce::AudioBuffer<float> swBuffer;
	juce::AudioBuffer<float> synthBuffer;
	juce::AudioBuffer<float> vaBuffer;
	juce::ADSR ampEnvelope;
	juce::ADSR modEnvelope;
	juce::ADSR::Parameters ampEnvelopeParameters;
	juce::ADSR::Parameters modEnvelopeParameters;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
	juce::ValueTree state;
};
