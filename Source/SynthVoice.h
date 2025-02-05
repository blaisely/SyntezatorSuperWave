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
class SynthVoice : public juce::SynthesiserVoice, juce::ValueTree::Listener
{
public:

    explicit SynthVoice::SynthVoice(juce::ValueTree& v):
    hiPassKeytrack(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(),20.0f)), state(v),
    oscSW{Osc(v),Osc(v)},oscVA(v),
    vaSVF(v),
    ladder(v),
    lfoGenerator1(v), lfoGenerator2(v), lfoGenerator3(v)
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
        midiNote = midiNoteNumber;
        resetLFO();
        setOscillatorsFrequency(midiNote,velocity);
        setRandomPhase();
        if(filterKeytrack)
        {
            vaSVF.setParameters(filterKeytrack,filterKeytrackOffset,midiNote);
            ladder.setParameters(filterKeytrack,filterKeytrackOffset,midiNote);
        }

        *hiPassKeytrack.state = *juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(),
        static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNote)));

        ampEnv.noteOn();
        amp2Env.noteOn();
        modEnv.noteOn();
        modEnv2.noteOn();
    }
    void setOscillatorsFrequency(const int midiNote, const float velocity)
    {
        float analogOffset =0;
        for(auto i=0;i<2;++i)
        {
            analogOffset = juce::Random::getSystemRandom().nextFloat()*0.008f;
            oscSW[i].setFrequency(frequency, midiNote, velocity, analogOffset+0.3f);
        }

        oscVA.setFrequency(frequency, midiNote, velocity,analogOffset);
    }
    void setRandomPhase()
    {
        phase = oscSW[kLeft].randomPhase();
        for(float & phase : phases)
        {
            phase = oscSW[kLeft].randomPhase();
        }
        oscSW[kLeft].setRandomPhase(phase,phases[0], phases[1], phases[2], phases[3], phases[4], phases[5]);
        oscSW[kRight].setRandomPhase(phase,phases[0], phases[1], phases[2], phases[3], phases[4], phases[5]);
        oscVA.setRandomPhase(phase);
    }

    void stopNote(float velocity, bool allowTailOff) override
    {
        ampEnv.noteOff();
        amp2Env.noteOff();
        modEnv.noteOff();
        modEnv2.noteOff();
        resetLFO();
        if(!env1OSC2)
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
        synthBuffer.setSize(2, samplesPerBlock, true, true, true);
        synthBuffer.clear();
        inverseSampleRate = 1.0f / sampleRate;
        const juce::dsp::ProcessSpec spec{sampleRate,static_cast<uint32_t>(samplesPerBlock),static_cast<uint32_t>(outputChannels)};
        level.prepare(spec);
        hiPassKeytrack.prepare(spec);
        ladder.prepare(spec);
        ladder.prepareSmoother(sampleRate);
        level.setGainLinear(0.5f);
        lfoGenerator1.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
        lfoGenerator2.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
        lfoGenerator3.prepareToPlay(sampleRate,samplesPerBlock,outputChannels);
        oscSW[kLeft].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
        oscSW[kRight].prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
        oscVA.prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
        panOSC1.reset(sampleRate,0.001);
        panOSC2.reset(sampleRate,0.001);
        modMatrix.addDestination(ModMatrix::modDestination::kFILTER_CUTOFFSVF,vaSVF.getModCutOff());
        modMatrix.addDestination(ModMatrix::modDestination::kFILTER_RESONANCE,vaSVF.getModResonance());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC_DETUNE,oscSW[kLeft].getModDetune());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC_VOLUME,oscSW[kLeft].getModVolume());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC1_PITCH,oscSW[kLeft].getModPitch());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC1_GAIN,oscSW[kLeft].getModGain());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC2_GAIN,oscVA.getModGain());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC2_PITCH,oscVA.getModPitch());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC1_PAN,&panMod1);
        modMatrix.addDestination(ModMatrix::modDestination::kOSC2_PAN,&panMod2);
        modMatrix.addDestination(ModMatrix::modDestination::kLFO1_AMT,lfoGenerator1.getModAmount());
        modMatrix.addDestination(ModMatrix::modDestination::kLFO1_FREQ,lfoGenerator1.getModFrequency());
        modMatrix.addDestination(ModMatrix::modDestination::kLFO2_AMT,lfoGenerator2.getModAmount());
        modMatrix.addDestination(ModMatrix::modDestination::kLFO3_AMT,lfoGenerator3.getModAmount());
        modMatrix.addDestination(ModMatrix::modDestination::kLFO2_FREQ,lfoGenerator2.getModFrequency());
        modMatrix.addDestination(ModMatrix::modDestination::kLFO3_FREQ,lfoGenerator3.getModFrequency());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC1_PWM,oscSW[kLeft].getModPWM());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC1_TYPE,oscSW[kLeft].getModOscType());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC2_TYPE,oscVA.getModOscType());
        modMatrix.addDestination(ModMatrix::modDestination::kOSC2_PWM,oscVA.getModPWM());
        modMatrix.addSource(ModMatrix::modSource::kLFO,&lfo1Mod);
        modMatrix.addSource(ModMatrix::modSource::kEG,&nextModEnv1);
        modMatrix.addSource(ModMatrix::modSource::kEG2,&nextModEnv2);
        modMatrix.addSource(ModMatrix::modSource::kLFO2,&lfo2Mod);
        modMatrix.addSource(ModMatrix::modSource::kLFO3,&lfo3Mod);
        modMatrix.addSource(ModMatrix::modSource::kAMP,&nextAmpSample);
        reset();
        isPrepared = true;
    }

    void update()
    {
        setKeytrack();
        setLFOParameters();
        getEnvelopeParameters();
        setFilterParameters();
        setOscParameters();
        setPanParameters();
        updatePan();
        setModMatrix();
    }
    void setKeytrack()
    {
        filterKeytrack = state[IDs::FilterKeytrackEnable];
        filterKeytrackOffset = state[IDs::FilterKeytrackOffset];
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

        routing[4].modDest = state[IDs::ModDestination5];
        routing[4].modSource = state[IDs::ModSource5];
        routing[4].modIntensity = static_cast<float>(state[IDs::ModIntensity5])/100.f;

        routing[5].modDest = state[IDs::ModDestination6];
        routing[5].modSource = state[IDs::ModSource6];
        routing[5].modIntensity = static_cast<float>(state[IDs::ModIntensity6])/100.f;

        setModRouting();
    }
    void setModRouting()
    {
        for (auto i = 0; i < 6; i++) // 6 modulation slots
        {
            bool hasSourceChanged = oldRouting[i].modSource != routing[i].modSource;
            bool hasDestChanged = oldRouting[i].modDest != routing[i].modDest;
            bool hasIntensityChanged = oldRouting[i].modIntensity != routing[i].modIntensity;
            // when it is connected
            if (routing[i].modDest > 0)
            {
                if (hasSourceChanged || hasDestChanged || hasIntensityChanged)
                {
                    modMatrix.addRouting(routing[i].modSource, routing[i].modDest-1, routing[i].modIntensity);

                    // if destination or source has changed reset routing
                    if (hasDestChanged || hasSourceChanged)
                    {
                        if(oldRouting[i].modDest>0)
                            modMatrix.resetRouting(oldRouting[i].modSource, oldRouting[i].modDest-1);
                    }

                    oldRouting[i].modDest = routing[i].modDest;
                    oldRouting[i].modSource = routing[i].modSource;
                    oldRouting[i].modIntensity = routing[i].modIntensity;
                }
            }
            else // when "no connection" is selected
            {
                if (oldRouting[i].modDest > 0) // if previous destination was connected
                    modMatrix.resetRouting(oldRouting[i].modSource, oldRouting[i].modDest-1);
                oldRouting[i].modDest = routing[i].modDest;
                oldRouting[i].modSource = routing[i].modSource;
                oldRouting[i].modIntensity = routing[i].modIntensity;
            }
        }
    }
    void setFilterParameters()
    {
        vaSVF.setParameters(filterKeytrack,filterKeytrackOffset,midiNote);
        ladder.setParameters(filterKeytrack,filterKeytrackOffset,midiNote);
        SVFEnabled = static_cast<int>(state[IDs::SVFEnabled]);
    }
    void setOscParameters()
    {
        oscSW[kLeft].setParameters();
        oscSW[kRight].setParameters();
        oscVA.setParameters();
    }
    void getEnvelopeParameters()
    {
        ampEnv.setParameters(state[IDs::ADSR1Attack],state[IDs::ADSR1Decay],state[IDs::ADSR1Sustain],state[IDs::ADSR1Release]);
        amp2Env.setParameters(state[IDs::ADSR1Attack],state[IDs::ADSR1Decay],state[IDs::ADSR1Sustain],state[IDs::ADSR1Release]);
        modEnv.setParameters(state[IDs::ADSR2Attack],state[IDs::ADSR2Decay],state[IDs::ADSR2Sustain],state[IDs::ADSR2Release]);
        modEnv2.setParameters(state[IDs::ADSR3Attack],state[IDs::ADSR3Decay],state[IDs::ADSR3Sustain],state[IDs::ADSR3Release]);

        envelopeAmount = static_cast<float>(state[IDs::FilterEnvelopeAmount])/100.f;
        envelopeAmount2 = static_cast<float>(state[IDs::FilterEnvelopeAmount2])/100.f;
        env1OSC2 = state[IDs::CommonEnvelope];
        loopEnvelope = state[IDs::LoopEnvelope];
        loopEnvelope2 = state[IDs::LoopEnvelope2];
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

        float depthLfo3 =static_cast<float>(state[IDs::LFO3Depth])/100.f;
        float freqLfo3 = state[IDs::LFO3Freq];
        int typeLfo3 = state[IDs::LFO3Type];

        bool lfo1Unipolar = state[IDs::LFO1Unipolar];
        bool lfo2Unipolar = state[IDs::LFO2Unipolar];
        bool lfo3Unipolar = state[IDs::LFO3Unipolar];
        lfo1Reset = state[IDs::LFOReset];
        lfo2Reset = state[IDs::LFOReset2];
        lfo3Reset = state[IDs::LFOReset3];

        lfoGenerator1.setParameters(depthLfo1,freqLfo1,typeLfo1,lfo1Unipolar);
        lfoGenerator2.setParameters(depthLfo2,freqLfo2,typeLfo2,lfo2Unipolar);
        lfoGenerator3.setParameters(depthLfo3,freqLfo3,typeLfo3,lfo3Unipolar);
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
                vaSVF.calculateFilterCoeffs();
                ladder.updateModulation();
                ladder.updateSmoothing();
                updatePan();
                float channelLeft = 0.0f;
                float channelRight = 0.0f;

                nextAmpSample = ampEnv.nextValue();
                nextAmp2Sample = amp2Env.nextValue();
                nextModEnv1 = modEnv.nextValue()*envelopeAmount;
                nextModEnv2 = modEnv2.nextValue()*envelopeAmount2;

                float osc2Output = oscVA.getNextSample();
                float oscSWOutL = oscSW[kLeft].getNextSample();
                float oscSWOutR = oscSW[kRight].getNextSample();

                loopModEnvelope1();
                loopModEnvelope2();

                if(!env1OSC2)
                {
                    channelLeft += osc2Output * nextAmpSample * panLeft[1];
                    channelRight += osc2Output * nextAmpSample * panRight[1];
                }
                else
                {
                    channelLeft += osc2Output * nextAmp2Sample * panLeft[1];
                    channelRight += osc2Output * nextAmp2Sample * panRight[1];
                }
                channelLeft += oscSWOutL * nextAmpSample * panLeft[0];
                channelRight += oscSWOutR * nextAmpSample * panRight[0];
                if (SVFEnabled)
                {
                    processSVF(channelLeft, channelRight);
                }
                else
                {
                    processLadder(channelLeft, channelRight);
                }

                processGain(channelLeft, channelRight);

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
                lfo3Mod = lfoGenerator3.render();
                modMatrix.render();
                ladder.setModResonance(*vaSVF.getModResonance());
                ladder.setModCutOff(*vaSVF.getModCutOff());

                oscSW[kRight].setModValue(*oscSW[kLeft].getModDetune(),0);
                oscSW[kRight].setModValue(*oscSW[kLeft].getModVolume(),1);
                oscSW[kRight].setModValue(*oscSW[kLeft].getModPitch(),2);
                oscSW[kRight].setModValue(*oscSW[kLeft].getModGain(),3);
                oscSW[kRight].setModValue(*oscSW[kLeft].getModPWM()+modDetune,4);
                oscSW[kRight].setModValue(*oscSW[kLeft].getModOscType()+modDetune,5);
            }
        }
        processKeytrack(oscillatorSW);

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addFrom(channel, startSample, swBuffer, channel, 0, numSamples);
        }
        if(env1OSC2)
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

    void setUpBuffer(const juce::AudioBuffer<float>& outputBuffer,const int numSamples)
    {
        swBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
        swBuffer.clear();
    }
    void processKeytrack(juce::dsp::AudioBlock<float> oscillatorSW)
    {
        juce::dsp::ProcessContextReplacing<float> process_context_replacing{oscillatorSW};
        hiPassKeytrack.process(process_context_replacing);
    }
    void processSVF(float& channelLeft, float& channelRight)
    {
        channelLeft = vaSVF.processAudioSample(channelLeft, 0);
        channelRight = vaSVF.processAudioSample(channelRight, 1);
    }

    void processLadder(float& channelLeft, float& channelRight)
    {
        channelLeft = ladder.processAudioSample(channelLeft, 0);
        channelRight = ladder.processAudioSample(channelRight, 1);
    }

    void processGain(float& channelLeft, float& channelRight)
    {
        channelLeft = level.processSample(channelLeft);
        channelRight = level.processSample(channelRight);
    }

    void loopModEnvelope1()
    {
        if(modEnv1Loop())
        {
            modEnv.reset();
            modEnv.noteOn();
        }
    }

    void loopModEnvelope2()
    {
        if(modEnv2Loop())
        {
            modEnv2.reset();
            modEnv2.noteOn();
        }
    }
    bool modEnv1Loop()
    {
        return modEnv.isInSustain() && loopEnvelope;
    }
    bool modEnv2Loop()
    {
        return modEnv2.isInSustain() && loopEnvelope2;
    }

    void reset()
    {
        resetOscillators();
        vaSVF.reset(getSampleRate());
        resetLFO();
        level.reset();
        hiPassKeytrack.reset();
        ladder.reset();
        ampEnv.reset();
        amp2Env.reset();
        modEnv.reset();
        panOSC1.reset(getSampleRate(),0.001);
        panOSC2.reset(getSampleRate(),0.001);
    }
    void resetLFO()
    {
        if(lfo1Reset)
            lfoGenerator1.reset();
        if(lfo2Reset)
            lfoGenerator2.reset();
        if(lfo3Reset)
            lfoGenerator3.reset();
    }
    void resetOscillators()
    {
        oscSW[kLeft].resetOsc();
        oscSW[kRight].resetOsc();
        oscVA.resetOsc();
    }
    void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
    {

    }

    static constexpr int numChannelsToProcess{2};

private:
    float modDetune=0.2f;
    enum
    {
        kLeft,
        kRight
    };
    struct modRouting
    {
        int modDest=0;
        int modSource=0;
        float modIntensity=0.f;
    };
    float inverseSampleRate{0.f};
    int midiNote{0};
    bool filterKeytrack{false};
    int filterKeytrackOffset{0};
    std::array<modRouting,6> routing;
    std::array<modRouting,6> oldRouting;
    float nextAmpSample{0.f};
    float nextAmp2Sample{0.f};
    juce::SmoothedValue<float> panOSC1{0.0f};
    juce::SmoothedValue<float> panOSC2{0.0f};
    float panLeft[2]{0.f};
    float panRight[2]{0.f};
    float lfo1Mod{0.0f};
    float lfo2Mod{0.0f};
    float lfo3Mod{0.0f};
    float envelopeAmount{0.0f};
    float envelopeAmount2{0.0f};
    float nextModEnv1{0.0f};
    float nextModEnv2{0.0f};
    bool isPrepared{false};
    std::array<float, 6> phases{0.0f};
    float phase{ 0.0f };
    double frequency{};
    float panMod1{0.0f};
    float panMod2{0.0f};
    bool SVFEnabled;
    bool env1OSC2;
    bool lfo1Reset;
    bool lfo2Reset;
    bool lfo3Reset;
    bool loopEnvelope = false;
    bool loopEnvelope2 = false;
    float oldFrequency{ 0.0f };
    std::array<Osc,2> oscSW;
    //Osc  oscSW;
    VAOsc oscVA;
    ModMatrix modMatrix;
    juce::dsp::Gain<float> level;
    ZVAFilter vaSVF;
    MOOGFilter ladder;
    LFO lfoGenerator1;
    LFO lfoGenerator2;
    LFO lfoGenerator3;
    juce::AudioBuffer<float> swBuffer;
    juce::AudioBuffer<float> synthBuffer;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  hiPassKeytrack;
    juce::ValueTree state;
    analogEG ampEnv;
    analogEG amp2Env;
    analogEG modEnv;
    analogEG modEnv2;
    int updateRate{ 32 };
    int updateCounter{ updateRate };
};
