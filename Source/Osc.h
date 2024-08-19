
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
#define MOD 1000000007

class Osc  final :juce::ValueTree::Listener
{
public:
    explicit Osc(juce::ValueTree& v) :state(v)
    {

        state.addListener(this);

    }
    static double OscType(const int& oscillatorType,const double& phase) {
        switch (oscillatorType) {
        case 0: return sine(phase);
        case 1: return poly_saw(phase);
        case 2: return square(phase);
        case 3: return triangle(phase);
        default: return poly_saw(phase);
        }
    }

    static double poly_blep(double t, const double& phaseIncrement) {
        const double dt = phaseIncrement / juce::MathConstants<float>::twoPi;
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.;
        }
        else if (t > 1.f - dt) {
            t = (t - 1.f) / dt;
            return t * t + t + t + 1.;
        }
        else {
            return 0.;
        }
    }

    static double poly_saw(const double& phase) {
        const double value = (2.0 * phase / juce::MathConstants<float>::twoPi) - 1.0;
        return value;
    }

    static double sine(const double& phase) {
        return sin(phase);
    }

    static double square(const double& phase) {
        return (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
    }

    static double triangle(const double& phase) {
        const double& value = -1.0f + (2.0f * phase / juce::MathConstants<float>::twoPi);
        return 2.0f * (fabs(value) - 0.5f);
    }
    void getNextBlock(juce::dsp::AudioBlock<float>& block, const int channel)
    {
        auto numSamples = block.getNumSamples();
        auto* sample = block.getChannelPointer(channel);
        
        for (size_t i = 0; i < numSamples; i++)
        {
            sample[i] = nextSampleUniversal(params.phases[0], params.phaseIncrements[0],
                params.lastOutputs[0]) * params.volumeMain;
             sample[i] += nextSampleUniversal(params.phases[1], params.phaseIncrements[1],
                 params.lastOutputs[1]) * params.volumeSides;
             sample[i] += nextSampleUniversal(params.phases[2], params.phaseIncrements[2],
                 params.lastOutputs[2]) * params.volumeSides;
             sample[i] += nextSampleUniversal(params.phases[3], params.phaseIncrements[3],
                 params.lastOutputs[3]) * params.volumeSides;
             sample[i] += nextSampleUniversal(params.phases[4], params.phaseIncrements[4],
                 params.lastOutputs[4]) * params.volumeSides;
             sample[i] += nextSampleUniversal(params.phases[5], params.phaseIncrements[5],
                 params.lastOutputs[5]) * params.volumeSides;
             sample[i] += nextSampleUniversal(params.phases[6], params.phaseIncrements[6],
                 params.lastOutputs[6]) * params.volumeSides;

            sample[i] = gain.processSample(sample[i]);

        }
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
    static float randomPhase()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(0.0, 1.0);
        auto randomValue = static_cast<float> (distr(gen));
        return randomValue;
    }
    void setRandomPhase(const double& phase, const double&  phaseOsc1, const double& phaseOsc2,const double& phaseOsc3,
        const double& phaseOsc5, const double& phaseOsc6,const double& phaseOsc7)
    {
        params.phases[0] = phase;
        params.phases[1] = phaseOsc1;
        params.phases[2] = phaseOsc2;
        params.phases[3] = phaseOsc3;
        params.phases[4] = phaseOsc5;
        params.phases[5] = phaseOsc6;
        params.phases[6] = phaseOsc7;
    }
    float nextSampleUniversal(double& phase,const double& phaseIncrement, double& lastOutput) const {
        const double t = phase / juce::MathConstants<float>::twoPi;
        double value = 0.0f;

        switch (static_cast<int>(state[IDs::SWtype])) {
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

        return static_cast<float>(value);
    }
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels) {
        juce::dsp::ProcessSpec spec{};
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        gain.prepare(spec);
        keyTrack.prepare(spec);
        lastSampleRate = sampleRate;
    }
    void setFrequency(const double& frequency,const int midiNote)
    {
        midiPitch = juce::MidiMessage::getMidiNoteInHertz(midiNote);
       updatePitch();
    }
    void updatePitch()
    {
        const double modPitch = std::pow(2.0,(static_cast<int>(state[IDs::SWoctave]) * 12 + static_cast<int>(state[IDs::SWdetune]))/12);
        const double freq = midiPitch * modPitch;
        this->oscFrequency = freq;
        const double phaseInc = freq / lastSampleRate;
        params.phaseIncrements[0] = phaseInc;
        setSidePhase();
    }

    void setSideOsc(const float detune,const float volume)
    {
        params.detune = polyFit(detune);
        params.volumeSides = sideVolume(volume);
        params.volumeMain = mainVolume(volume);
    }
    void setSidePhase()
    {
        params.phaseIncrements[1] = ((oscFrequency * (1 - (0.11002313 * params.detune))) / lastSampleRate);
        params.phaseIncrements[2] = ((oscFrequency * (1 - (0.06288439 * params.detune))) / lastSampleRate);
        params.phaseIncrements[3] = ((oscFrequency * (1 - (0.01952356 * params.detune))) / lastSampleRate);
        params.phaseIncrements[4] =((oscFrequency * (1 + (0.01991221 * params.detune))) / lastSampleRate);
        params.phaseIncrements[5] = ((oscFrequency * (1 + (0.06216538 * params.detune))) / lastSampleRate);
        params.phaseIncrements[6] = ((oscFrequency * (1 + (0.10745242 * params.detune))) / lastSampleRate);
    }
    static float polyFit(float x)
    {
        //TODO can be approximated with 3 functions
        float y{ 0.0f };
        y = (10028.7312891634f * fast_power(x, 11)) - (50818.8652045924f * fast_power(x, 10)) + (111363.4808729368f * fast_power(x, 9)) -
            (138150.6761080548 * fast_power(x, 8)) + (106649.6679158292 * fast_power(x, 7)) - (53046.9642751875f * fast_power(x, 6)) + (17019.9518580080 * fast_power(x, 5)) -
            (3425.0836591318 * fast_power(x, 4)) + (404.2703938388 * fast_power(x, 3)) - (24.1878824391 * fast_power(x, 2)) + (0.6717417634 * x) + 0.0030115596;
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
    static float fast_power(float base, int power) {
        float result = 1.0f;
        while (power > 0) {

            if (power % 2 == 1) {
                result = std::fmod((result * base), MOD);
            }
            base = std::fmod((base * base), MOD);
            power = power / 2;
        }
        return result;
    }
    void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
    {
        if (v == state)
        {
                if (id == IDs::SWdetuneS || id == IDs::SWvolumeS) {
                    setSideOsc(state[IDs::SWdetuneS], state[IDs::SWvolumeS]);
                }
                if (id == IDs::SWoctave) {
                    updatePitch();
                }
                if (id == IDs::SWdetune) {
                    updatePitch();
                }
                if (id == IDs::SWtype) {
                    type = state.getProperty(IDs::SWtype);
                }
                if (id == IDs::SWgain) {
                    gain.setGainLinear(state[IDs::SWgain]);
                }
                if (id == IDs::SWFrequency)
                {
                    setFrequency(state[IDs::SWFrequency], state[IDs::MidiNote]);
                }


        }
    }
private:
    juce::ValueTree state;
    struct synthParams
    {
        float detune{ 0.0f };
        float volumeSides{ 0.0f };
        float volumeMain{ 0.0f };
        std::array<double, 7> phaseIncrements{0.0f};
        std::array<double, 7> phases{};
        std::array<double, 7> lastOutputs{ 0.0f };
    };
    synthParams params;
    float type{ 0.0f };
    juce::dsp::Gain<float> gain;
    double midiPitch{0.f};
    double lastSampleRate{0.0f};
    double oscFrequency{ 0.0f };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
};
