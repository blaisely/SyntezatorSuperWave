
/*
  ==============================================================================

    Osc.h
    Created: 8 Apr 2024 5:56:11pm
    Author:  blaze

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include<random>
#include "SharedData.h"
#include "helpers.h"


class Osc  final :juce::ValueTree::Listener
{
public:
    explicit Osc(juce::ValueTree& v) :state(v)
    {
    }
    void prepareToPlay(const float& sampleRate,const int& samplesPerBlock,const int& outputChannels) {
        juce::dsp::ProcessSpec spec{};
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        gain.prepare(spec);
        keyTrack.prepare(spec);
        lastSampleRate = sampleRate;
    }

    void getNextBlock(juce::dsp::AudioBlock<float>& block, const int channel)
    {
        auto numSamples = block.getNumSamples();
        auto* sample = block.getChannelPointer(channel);
        
        for (size_t i = 0; i < numSamples; i++)
        {
            sample[i] = nextSample(params.phases[0], params.phaseIncrements[0],
                params.lastOutputs[0]) * params.volumeMain;
             sample[i] += nextSample(params.phases[1], params.phaseIncrements[1],
                 params.lastOutputs[1]) * params.volumeSides;
             sample[i] += nextSample(params.phases[2], params.phaseIncrements[2],
                 params.lastOutputs[2]) * params.volumeSides;
             sample[i] += nextSample(params.phases[3], params.phaseIncrements[3],
                 params.lastOutputs[3]) * params.volumeSides;
             sample[i] += nextSample(params.phases[4], params.phaseIncrements[4],
                 params.lastOutputs[4]) * params.volumeSides;
             sample[i] += nextSample(params.phases[5], params.phaseIncrements[5],
                 params.lastOutputs[5]) * params.volumeSides;
             sample[i] += nextSample(params.phases[6], params.phaseIncrements[6],
                 params.lastOutputs[6]) * params.volumeSides;

            sample[i] = gain.processSample(sample[i]);
            sample[i] = sample[i]*0.5f;

        }
    }

    float getNextSample()
    {
        float y=0;
        y = nextSample(params.phases[0], params.phaseIncrements[0],
               params.lastOutputs[0]) * params.volumeMain;
        y += nextSample(params.phases[1], params.phaseIncrements[1],
            params.lastOutputs[1]) * params.volumeSides;
        y += nextSample(params.phases[2], params.phaseIncrements[2],
            params.lastOutputs[2]) * params.volumeSides;
        y += nextSample(params.phases[3], params.phaseIncrements[3],
            params.lastOutputs[3]) * params.volumeSides;
        y += nextSample(params.phases[4], params.phaseIncrements[4],
            params.lastOutputs[4]) * params.volumeSides;
        y += nextSample(params.phases[5], params.phaseIncrements[5],
            params.lastOutputs[5]) * params.volumeSides;
        y += nextSample(params.phases[6], params.phaseIncrements[6],
            params.lastOutputs[6]) * params.volumeSides;

        y = gain.processSample(y);
        y = y*0.5f;

        return y;
    }
    float nextSample(float& phase,const float& phaseIncrement, float& lastOutput) const {
        const float t = phase / juce::MathConstants<float>::twoPi;
        float value = 0.0f;

        switch (type) {
        case 0:
            value = sine(phase);
            break;
        case 1:
            value = poly_saw(phase);
            value -= poly_blep(t, phaseIncrement);
            break;
        case 2:
            value = square(phase);
            value += poly_blep(t, phaseIncrement);
            value -= poly_blep(fmod(t + 0.5f, 1.0f), phaseIncrement);
            break;
        case 3:
            value = triangle(phase);
            value += poly_blep(t, phaseIncrement);
            value -= poly_blep(fmod(t + 0.5f, 1.0f), phaseIncrement);
            value = phaseIncrement * value + (1.0f - phaseIncrement) * lastOutput;
            lastOutput = value;
            break;
            default:
                value = sine(phase);
        	break;
        }

        phase += phaseIncrement;
        while (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        return value;
    }
    static float poly_blep(float t, const float& phaseIncrement) {
        const float dt = phaseIncrement / juce::MathConstants<float>::twoPi;
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.f;
        }
        else if (t > 1.f - dt) {
            t = (t - 1.f) / dt;
            return t * t + t + t + 1.f;
        }
        else {
            return 0.;
        }
    }

    static float poly_saw(const float& phase) {
        const float value = (2.0f * phase / juce::MathConstants<float>::twoPi) - 1.0f;
        return value;
    }

    static float sine(const float& phase) {
        return sin(phase);
    }

    static float square(const float& phase) {
        return (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
    }

    static float triangle(const float& phase) {
        const float& value = -1.0f + (2.0f * phase / juce::MathConstants<float>::twoPi);
        return 2.0f * (fabs(value) - 0.5f);
    }
    static float OscType(const int& oscillatorType,const float& phase) {
        switch (oscillatorType) {
        case 0: return sine(phase);
        case 1: return poly_saw(phase);
        case 2: return square(phase);
        case 3: return triangle(phase);
        default: return poly_saw(phase);
        }
    }
    void setFrequency(const float& frequency,const int midiNote)
    {
        midiPitch =static_cast<float>( juce::MidiMessage::getMidiNoteInHertz(midiNote));
        updatePitch();
    }
    void updatePitch()
    {
        const float modPitch = std::pow(2.0f,(octave + detuneSemi)/12.f);
        const float freq = midiPitch * modPitch;
        this->oscFrequency = freq;
        params.phaseIncrements[0] = (oscFrequency / lastSampleRate) * juce::MathConstants<float>::twoPi;
        setSidePhase();
    }
    static float randomPhase()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(0.0, 0.5);
        auto randomValue = static_cast<float> (distr(gen));
        return randomValue;
    }
    void setRandomPhase(const float& phase, const float&  phaseOsc1, const float& phaseOsc2,const float& phaseOsc3,
        const float& phaseOsc5, const float& phaseOsc6,const float& phaseOsc7)
    {
        params.phases[0] = phase;
        params.phases[1] = phaseOsc1;
        params.phases[2] = phaseOsc2;
        params.phases[3] = phaseOsc3;
        params.phases[4] = phaseOsc5;
        params.phases[5] = phaseOsc6;
        params.phases[6] = phaseOsc7;
    }
    void setSideOsc(const float detune,const float volume)
    {
        params.detune = polyFit(detune);
        params.volumeSides = sideVolume(volume);
        params.volumeMain = mainVolume(volume);
    }
    void setSidePhase()
    {
        params.phaseIncrements[1] = ((oscFrequency * (1 - (0.11002313f * params.detune))) / lastSampleRate* juce::MathConstants<float>::twoPi);
        params.phaseIncrements[2] = ((oscFrequency * (1 - (0.06288439f * params.detune))) / lastSampleRate* juce::MathConstants<float>::twoPi);
        params.phaseIncrements[3] = ((oscFrequency * (1 - (0.01952356f * params.detune))) / lastSampleRate* juce::MathConstants<float>::twoPi);
        params.phaseIncrements[4] =((oscFrequency * (1 + (0.01991221f * params.detune))) / lastSampleRate* juce::MathConstants<float>::twoPi);
        params.phaseIncrements[5] = ((oscFrequency * (1 + (0.06216538f * params.detune))) / lastSampleRate* juce::MathConstants<float>::twoPi);
        params.phaseIncrements[6] = ((oscFrequency * (1 + (0.10745242f * params.detune))) / lastSampleRate* juce::MathConstants<float>::twoPi);
    }
    static float polyFit(float x)
    {
        float y =0.0f;
        if(x>0 && x<5.4f)
            y=0.2f*x;
        if(x>=5.4f&&x<94)
            y=0.44f*x*x+0.1f*x-0.07f;
        if(x>=0.94f && x<=1)
            y=9.6f*x*x-8.5f*x;
        return y;
    }
    static float sideVolume(float x)
    {
        float y{ 0.0f };
        y = -0.73764f * fast_power(x, 2) + 1.2841f * x + 0.044372f;
        return y;
    }
    static float mainVolume(float x)
    {
        float y{ 0.0f };
        y = -0.55366f * x + 0.99785f;
        return y;
    }
    void setParameters()
    {
        setSideOsc(state[IDs::SWdetuneS], state[IDs::SWvolumeS]);
        octave = static_cast<float>(state[IDs::SWoctave])*12;
        detuneSemi = static_cast<float>(state[IDs::SWdetune])/12;
        updatePitch();
        type = state.getProperty(IDs::SWtype);
        gain.setGainLinear(state[IDs::SWgain]);
    }
    void resetOsc() {
        gain.reset();
        keyTrack.reset();
        for (auto i = 0; i < 7; i++)
        {
            params.phases[i] = 0.0f;
            params.phaseIncrements[i] = 0.0f;
        }
    }

private:
    juce::ValueTree state;
    struct synthParams
    {
        float detune{ 0.0f };
        float volumeSides{ 0.0f };
        float volumeMain{ 0.0f };
        std::array<float, 7> phaseIncrements{0.0f};
        std::array<float, 7> phases{};
        std::array<float, 7> lastOutputs{ 0.0f };
    };
    synthParams params;
    int type{ 0 };
    juce::dsp::Gain<float> gain;
    float octave{0};
    float detuneSemi{0};
    float midiPitch{0.f};
    float lastSampleRate{0.0f};
    float oscFrequency{ 0.0f };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
};
