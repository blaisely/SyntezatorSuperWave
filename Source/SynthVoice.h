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
#include "ModMatrix.h"
#include "SharedData.h"
class SynthVoice : public juce::SynthesiserVoice,juce::ValueTree::Listener
{
public:

	explicit SynthVoice::SynthVoice(juce::ValueTree& v):
	keyTrack(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000.0f,20.0f)), state(v),
	oscSW(v),oscVA(v),
	vaSVF(v),
	ladder(v),
	lfoGenerator1(v), lfoGenerator2(v)
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
			oscSW.setFrequency(frequency, midiNote);
			oscVA.setFrequency(frequency, midiNote);

	}
	void setRandomPhase()
	{
		phase = oscSW.randomPhase();
		for(float & phase : phases)
		{
			phase = oscSW.randomPhase();
		}

			oscSW.setRandomPhase(phase,phases[0], phases[1], phases[2], phases[3], phases[4], phases[5]);
			oscVA.setRandomPhase(phase);

	}

	void stopNote(float velocity, bool allowTailOff) override
	{
		ampEnv.noteOff();
		amp2Env.noteOff();
		modEnv.noteOff();
		resetLFO();
		if(commonEnvelope)
		{
			if (! allowTailOff || ! ampEnv.isActive() )
				clearCurrentNote();

		}
		else
		{
			if (! allowTailOff || ! ampEnv.isActive() && ! amp2Env.isActive()  )
				clearCurrentNote();

		}

	}
	void pitchWheelMoved(int newPitchWheelValue) override
	{
	}

	void controllerMoved(int controllerNumber, int newControllerValue) override
	{
	}

	void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
	{
		reset();
		synthBuffer.setSize(2, samplesPerBlock, true, true, true);
		synthBuffer.clear();
		const juce::dsp::ProcessSpec spec{sampleRate,static_cast<uint32_t>(samplesPerBlock),static_cast<uint32_t>(outputChannels)};
		level.prepare(spec);
		keyTrack.prepare(spec);
		ladder.prepare(spec);
		level.setGainLinear(0.5f);
		lfoGenerator1.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
		lfoGenerator2.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
		oscSW.prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
		oscVA.prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
		panOSC1.reset(sampleRate,0.001);
		panOSC2.reset(sampleRate,0.001);
		modMatrix.addDestination(ModMatrix::modDestination::kFILTER_CUTOFFLDDR,ladder.getModCutOff());
		modMatrix.addDestination(ModMatrix::modDestination::kFILTER_CUTOFFSVF,vaSVF.getModCutOff());
		modMatrix.addDestination(ModMatrix::modDestination::kFILTER_RESONANCE,vaSVF.getModResonance());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC_DETUNE,oscSW.getModDetune());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC_VOLUME,oscSW.getModVolume());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC1_PITCH,oscSW.getModPitch());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC1_GAIN,oscSW.getModGain());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC2_GAIN,oscVA.getModGain());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC2_PITCH,oscVA.getModPitch());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC1_PAN,&panMod1);
		modMatrix.addDestination(ModMatrix::modDestination::kOSC2_PAN,&panMod2);
		modMatrix.addDestination(ModMatrix::modDestination::kLFO1_AMT,lfoGenerator1.getModAmount());
		modMatrix.addDestination(ModMatrix::modDestination::kLFO1_FREQ,lfoGenerator1.getModFrequency());
		modMatrix.addDestination(ModMatrix::modDestination::kLFO2_AMT,lfoGenerator2.getModAmount());
		modMatrix.addDestination(ModMatrix::modDestination::kLFO2_FREQ,lfoGenerator2.getModFrequency());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC1_PWM,oscSW.getModPWM());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC1_TYPE,oscSW.getModOscType());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC2_TYPE,oscVA.getModOscType());
		modMatrix.addDestination(ModMatrix::modDestination::kOSC2_PWM,oscVA.getModPWM());
		modMatrix.addSource(ModMatrix::modSource::kLFO,&lfo1Mod);
		modMatrix.addSource(ModMatrix::modSource::kEG,&envelopeMod);
		modMatrix.addSource(ModMatrix::modSource::kLFO2,&lfo2Mod);
		modMatrix.addSource(ModMatrix::modSource::kAMP,&nextAmpSample);

		isPrepared = true;
	}

	void update()
	{
		setLFOParameters();
		getEnvelopeParameters();
		setFilterParameters();
		setOscParameters();
		setPanParameters();
		updatePan();
		setModMatrix();
	}
	void setModMatrix()
	{
		routing[0].modDest = state[IDs::ModDestination1];
		routing[0].modSource = state[IDs::ModSource1];
		routing[0].modIntensity = static_cast<float>(state[IDs::ModIntensity1])/100.f;

		routing[1].modDest = state[IDs::ModDestination2];
		routing[1].modSource = state[IDs::ModSource2];
		routing[1].modIntensity = static_cast<float>(state[IDs::ModIntensity2])/100.f;

		routing[2].modDest = state[IDs::ModDestination3];
		routing[2].modSource = state[IDs::ModSource3];
		routing[2].modIntensity = static_cast<float>(state[IDs::ModIntensity3])/100.f;

		routing[3].modDest = state[IDs::ModDestination4];
		routing[3].modSource = state[IDs::ModSource4];
		routing[3].modIntensity = static_cast<float>(state[IDs::ModIntensity4])/100.f;

		setModRouting();
	}
	void setModRouting()
	{
    for (auto i = 0; i < 4; i++) // 4 modulation slots
    {
        bool hasSourceChanged = oldRouting[i].modSource != routing[i].modSource;
        bool hasDestChanged = oldRouting[i].modDest != routing[i].modDest;
        bool hasIntensityChanged = oldRouting[i].modIntensity != routing[i].modIntensity;

        // Debugging: Print the routing values

        // when it is connected
        if (routing[i].modDest > 0)
        {
            if (hasSourceChanged || hasDestChanged || hasIntensityChanged)
            {
                // if CutOff is selected route both filters
                if (routing[i].modDest == 1)
                {
                    modMatrix.addRouting(routing[i].modSource, 0, routing[i].modIntensity); // SVF Filter
                    modMatrix.addRouting(routing[i].modSource, 1, routing[i].modIntensity); // Ladder Filter

                    // Debugging: Confirmation of routing to filters

                }
                else if (routing[i].modDest > 1) // other
                {
                    modMatrix.addRouting(routing[i].modSource, routing[i].modDest, routing[i].modIntensity);

                    // Debugging: Confirmation of other routing

                }

                // if destination or source has changed reset routing
                if (hasDestChanged || hasSourceChanged)
                {
                    if (oldRouting[i].modDest == 1)
                    {
                        modMatrix.resetRouting(oldRouting[i].modSource, 0); // SVF Filter
                        modMatrix.resetRouting(oldRouting[i].modSource, 1); // Ladder Filter
                    }
                    else if (oldRouting[i].modDest > 1)
                    {
                        modMatrix.resetRouting(oldRouting[i].modSource, oldRouting[i].modDest);
                    }

                }

                oldRouting[i].modDest = routing[i].modDest;
                oldRouting[i].modSource = routing[i].modSource;
                oldRouting[i].modIntensity = routing[i].modIntensity;
            }
        }
        else // when "no connection" is selected
        {
            if (oldRouting[i].modDest > 0) // if previous destination was connected
            {
                if (oldRouting[i].modDest == 1)
                {
                    modMatrix.resetRouting(oldRouting[i].modSource, 0); // SVF Filter
                    modMatrix.resetRouting(oldRouting[i].modSource, 1); // Ladder Filter
                }
                else if (oldRouting[i].modDest > 1)
                {
                    modMatrix.resetRouting(oldRouting[i].modSource, oldRouting[i].modDest);
                }
            }

            oldRouting[i].modDest = routing[i].modDest;
            oldRouting[i].modSource = routing[i].modSource;
            oldRouting[i].modIntensity = routing[i].modIntensity;
        }
    }
}
	void setFilterParameters()
	{
		vaSVF.setParameters();
		ladder.setParameters();
		SVFEnabled = static_cast<int>(state[IDs::SVFEnabled]);
	}
	void setOscParameters()
	{
		oscSW.setParameters();
		oscVA.setParameters();
	}
	void getEnvelopeParameters()
	{
		auto sampleRate = float(getSampleRate());
		float inverseSampleRate = 1.0f / sampleRate;

		ampEnv.attackMultiplier = std::exp(-inverseSampleRate *
			std::exp(5.5f - 0.075f * static_cast<float>(state[IDs::ADSR1Attack])));
		ampEnv.decayMultiplier = std::exp(-inverseSampleRate *
			std::exp(5.5f - 0.075f * static_cast<float>(state[IDs::ADSR1Decay])));
		ampEnv.sustainLevel = static_cast<float>(state[IDs::ADSR1Sustain])/100.f;
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
		amp2Env.sustainLevel=modEnv.sustainLevel = static_cast<float>(state[IDs::ADSR2Sustain])/100.f;
		float env2Release = state[IDs::ADSR2Release];
		if (env2Release < 1.0f) {
			amp2Env.releaseMultiplier = modEnv.releaseMultiplier = 0.75f;
		} else {
			amp2Env.releaseMultiplier=modEnv.releaseMultiplier =
				std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * env2Release));
		}

		envelopeAmount = static_cast<float>(state[IDs::FilterEnvelopeAmount])/100.f;
		commonEnvelope = state[IDs::CommonEnvelope];
		loopEnvelope = state[IDs::LoopEnvelope];
	}
	void setPanParameters()
	{
		panOSC1.setTargetValue(std::clamp(static_cast<float>(state[IDs::PanOsc1])+panMod1,-1.f,1.f));
		panOSC2.setTargetValue(std::clamp(static_cast<float>(state[IDs::PanOsc2])+panMod2,-1.f,1.f));
	}
	void updatePan()
	{
		panLeft[0] = std::cos((juce::MathConstants<float>::halfPi/2.f)*(panOSC1.getNextValue()+1));
		panLeft[1] = std::cos((juce::MathConstants<float>::halfPi/2.f)*(panOSC2.getNextValue()+1));
		panRight[0] = std::sin((juce::MathConstants<float>::halfPi/2.f)*(panOSC1.getCurrentValue()+1));
		panRight[1] = std::sin((juce::MathConstants<float>::halfPi/2.f)*(panOSC2.getCurrentValue()+1));
	}
	void setLFOParameters()
	{
		float depthLfo1 =static_cast<float>(state[IDs::LFODepth])/100.f;
		float freqLfo1 = state[IDs::LFOFreq];
		int typeLfo1 = state[IDs::LFOType];
		float depthLfo2 =static_cast<float>(state[IDs::LFO2Depth])/100.f;
		float freqLfo2 = state[IDs::LFO2Freq];
		int typeLfo2 = state[IDs::LFO2Type];
		lfoGenerator1.setParameters(depthLfo1,freqLfo1,typeLfo1);
		lfoGenerator2.setParameters(depthLfo2,freqLfo2,typeLfo2);
		lfoReset = state[IDs::LFOReset];
	}
	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
{
    jassert(isPrepared);
    if (!isVoiceActive())
        return;

    setUpBuffer(outputBuffer, numSamples);
    juce::dsp::AudioBlock<float> oscillatorSW{swBuffer};

    auto inputLeft = swBuffer.getReadPointer(0);
    auto inputRight = swBuffer.getReadPointer(1);
    auto outputLeft = swBuffer.getWritePointer(0);
    auto outputRight = swBuffer.getWritePointer(1);

    int remainingSamples = numSamples;
    int samplePos = 0;

    while (remainingSamples > 0)
    {
        const int numToProcess = juce::jmin(remainingSamples, updateCounter);

        for (int sample = 0; sample < numToProcess; ++sample)
        {
		updatePan();
            float channelLeft = 0.0f;
            float channelRight = 0.0f;

            nextAmpSample = ampEnv.nextValue();
            nextAmp2Sample = amp2Env.nextValue();
        	envelopeMod = modEnv.nextValue()*envelopeAmount;

        	if (modEnv.isInSustain()==true && loopEnvelope)
        	{
        		modEnv.reset();
        		modEnv.noteOn();
        	}

        	float osc2Output = oscVA.getNextSample();
        	float osc1Output = oscSW.getNextSample();

			if(commonEnvelope)
			{
				channelLeft += osc2Output * nextAmpSample * panLeft[1];
				channelRight += osc2Output * nextAmpSample * panRight[1];
			}
        	else
        	{
        		channelLeft += osc2Output * nextAmp2Sample * panLeft[1];
        		channelRight += osc2Output * nextAmp2Sample * panRight[1];
        	}
            channelLeft += osc1Output * nextAmpSample * panLeft[0];
            channelRight += osc1Output * nextAmpSample * panRight[0];

            if (SVFEnabled)
            {
                channelLeft = vaSVF.processAudioSample(channelLeft, 0);
                channelRight = vaSVF.processAudioSample(channelRight, 1);
            }
            else
            {
                channelLeft = ladder.processAudioSample(channelLeft, 0);
                channelRight = ladder.processAudioSample(channelRight, 1);
            }

            channelLeft = level.processSample(channelLeft);
            channelRight = level.processSample(channelRight);

            outputLeft[samplePos + sample] = channelLeft + inputLeft[samplePos + sample];
            outputRight[samplePos + sample] = channelRight + inputRight[samplePos + sample];
        }

        remainingSamples -= numToProcess;
        samplePos += numToProcess;
        updateCounter -= numToProcess;

        if (updateCounter <= 0)
        {
            updateCounter = updateRate;
            lfo1Mod = lfoGenerator1.render();
        	lfo2Mod = lfoGenerator2.render();
            modMatrix.render();
        	ladder.setModResonance(*vaSVF.getModResonance());
            vaSVF.updateModulation();
            ladder.updateModulation();
        }
    }

    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom(channel, startSample, swBuffer, channel, 0, numSamples);
    }
		if(commonEnvelope)
		{
			if (!ampEnv.isActive()&&!amp2Env.isActive())
			{
				clearCurrentNote();
				resetOscillators();
			}
		}
		else
		{
			if (!ampEnv.isActive())
			{
				clearCurrentNote();
				resetOscillators();
			}
		}

}

	void updateModulations()
	{
		vaSVF.updateModulation();
		ladder.updateModulation();
	}

	void setUpBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
	{
		swBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
		swBuffer.clear();
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
			lfoGenerator1.reset();
			lfoGenerator2.reset();
		}
	}
	void resetOscillators()
	{

		oscSW.resetOsc();
		oscVA.resetOsc();
	}
	void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
	{}

	static constexpr int numChannelsToProcess{2};

private:
	struct modRouting
	{
		int modDest=0;
		int modSource=0;
		float modIntensity=0.f;
	};
	std::array<modRouting,4> routing;
	std::array<modRouting,4> oldRouting;
	float nextAmpSample{0.f};
	float nextAmp2Sample{0.f};
	juce::SmoothedValue<float> panOSC1{0.0f};
	juce::SmoothedValue<float> panOSC2{0.0f};
	float panLeft[2]{0.f};
	float panRight[2]{0.f};
	float lfo1Mod{0.0f};
	float lfo2Mod{0.0f};
	float envelopeAmount{0.0f};
	float envelopeMod{0.0f};
	bool isPrepared{false};
	std::array<float, 6> phases{0.0f};
	float phase{ 0.0f };
	double frequency{};
	float panMod1{0.0f};
	float panMod2{0.0f};
	bool SVFEnabled;
	bool commonEnvelope;
	bool lfoReset;
	bool loopEnvelope = false;
	float oldFrequency{ 0.0f };
	Osc  oscSW;
	VAOsc oscVA;
	ModMatrix modMatrix;
	juce::dsp::Gain<float> level;
	ZVAFilter vaSVF;
	MOOGFilter ladder;
	LFO lfoGenerator1;
	LFO lfoGenerator2;
	juce::AudioBuffer<float> swBuffer;
	juce::AudioBuffer<float> synthBuffer;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
	juce::ValueTree state;
	analogEG ampEnv;
	analogEG amp2Env;
	analogEG modEnv;
	int updateRate{ 32 };
	int updateCounter{ updateRate };

};
