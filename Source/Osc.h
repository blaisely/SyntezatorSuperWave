
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

class Osc :juce::ValueTree::Listener
{
public:
    Osc(juce::ValueTree& v) :state(v), keyTrack(juce::dsp::IIR::Coefficients<float>::makeHighPass(48000, 0.1f))
    {

        state.addListener(this);

    }
    double OscType(int oscillatorType, double phase) {
        switch (oscillatorType) {
        case 0: return sine(phase);
        case 1: return poly_saw(phase);
        case 2: return square(phase);
        case 3: return triangle(phase);
        default: return sine(phase);
        }
    }

    double poly_blep(double t, double phaseIncrement) {
        double dt = phaseIncrement / juce::MathConstants<float>::twoPi;
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.;
        }
        else if (t > 1. - dt) {
            t = (t - 1.) / dt;
            return t * t + t + t + 1.;
        }
        else {
            return 0.;
        }
    }

    double poly_saw(double phase) {
        double value = (2.0 * phase / juce::MathConstants<float>::twoPi) - 1.0;
        return value;
    }

    double sine(double phase) {
        return sin(phase);
    }

    double square(double phase) {
        return (phase < juce::MathConstants<double>::pi) ? 1.0 : -1.0;
    }

    double triangle(double phase) {
        double value = -1.0 + (2.0 * phase / juce::MathConstants<float>::twoPi);
        return 2.0 * (fabs(value) - 0.5);
    }
    void getNextBlock(juce::dsp::AudioBlock<float>& block, const int channel)
    {
        auto numSamples = block.getNumSamples();
        auto numChannels = block.getNumChannels();
        auto* sample = block.getChannelPointer(channel);
        auto* sample2 = block.getChannelPointer(1);
        //setKeyTrackedFreq();
        for (size_t i = 0; i < numSamples; i++)
        {
            sample[i] = nextSampleUniversal(superOscSettings.phases[0], superOscSettings.phaseIncrements[0], superOscSettings.lastOutputs[0]) /** superOscSettings.volumeMain*/;
            /* sample[i] += gain.processSample(nextSampleUniversal(superOscSettings.phases[1], superOscSettings.phaseIncrements[1], superOscSettings.lastOutputs[1]) * superOscSettings.volume);
             sample[i] += gain.processSample(nextSampleUniversal(superOscSettings.phases[2], superOscSettings.phaseIncrements[2], superOscSettings.lastOutputs[2]) * superOscSettings.volume);
             sample[i] += gain.processSample(nextSampleUniversal(superOscSettings.phases[3], superOscSettings.phaseIncrements[3], superOscSettings.lastOutputs[3]) * superOscSettings.volume);
             sample[i] += gain.processSample(nextSampleUniversal(superOscSettings.phases[4], superOscSettings.phaseIncrements[4], superOscSettings.lastOutputs[4]) * superOscSettings.volume);
             sample[i] += gain.processSample(nextSampleUniversal(superOscSettings.phases[5], superOscSettings.phaseIncrements[5], superOscSettings.lastOutputs[5]) * superOscSettings.volume);
             sample[i] += gain.processSample(nextSampleUniversal(superOscSettings.phases[6], superOscSettings.phaseIncrements[6], superOscSettings.lastOutputs[6]) * superOscSettings.volume);*/
    
            //sample[i] = gain.processSample(nextKeyTrackedsample());
        }
    }

    void resetOsc() {
        gain.reset();
        keyTrack.reset();
        for (auto i = 0; i < 7; i++)
        {
            superOscSettings.phases[i] = 0.0f;
            superOscSettings.phaseIncrements[i] = 0.0f;
        }
    }
    float randomPhase()
    {
        std::random_device rd;  // random number
        std::mt19937 gen(rd()); // seed
        std::uniform_real_distribution<> distr(0.0, 1.0); // range
        float randomValue = distr(gen);
        return randomValue;
    }
    void setRandomPhase(float phase, float phaseOsc1, float phaseOsc2, float phaseOsc3, float phaseOsc5, float phaseOsc6, float phaseOsc7)
    {
        superOscSettings.phases[0] = phase;
        superOscSettings.phases[1] = phaseOsc1;
        superOscSettings.phases[2] = phaseOsc2;
        superOscSettings.phases[3] = phaseOsc3;
        superOscSettings.phases[4] = phaseOsc5;
        superOscSettings.phases[5] = phaseOsc6;
        superOscSettings.phases[6] = phaseOsc7;
    }
    double nextSampleUniversal(double& phase, double& phaseIncrement, double& lastOutput) {
        double t = phase / juce::MathConstants<float>::twoPi;
        double value = 0.0;

        switch (static_cast<int>(type)) {
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
            value -= poly_blep(fmod(t + 0.5, 1.0), phaseIncrement);
            break;
        case 3:
            value = triangle(phase);
            value += poly_blep(t, phaseIncrement);
            value -= poly_blep(fmod(t + 0.5, 1.0), phaseIncrement);
            value = phaseIncrement * value + (1.0 - phaseIncrement) * lastOutput;
            lastOutput = value;
            break;
        }

        phase += phaseIncrement;
        while (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        return value;
    }
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels) {
        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        fmOsc.prepare(spec);
        gain.prepare(spec);
        keyTrack.prepare(spec);
        lastSampleRate = sampleRate;
    }
    void getPhaseIncrement(double frequency, int midiNote) {
        float freq = juce::MidiMessage::getMidiNoteInHertz(midiNote + octavePitchShift);
        freq += coarseDetune;
        oldFrequency = newFrequency;
        this->newFrequency = freq;
        superOscSettings.phaseIncrements[0] = frequency / lastSampleRate;
        lastFreq = freq;
        setSuperOscilllatorFrequency();
    }
    void setPitchMod(const int oct, const float detune)
    {
        octavePitchShift = (float)(oct * 12);
        coarseDetune = detune;
    }
    void setSuperOscillators(float detune, float volume)
    {
        superOscSettings.detune = polyFit(detune);
        superOscSettings.volume = sideVolume(volume);
        superOscSettings.volumeMain = mainVolume(volume);
    }
    void setSuperOscilllatorFrequency()
    {
        superOscSettings.phaseIncrements[1] = (double)(((newFrequency * (1 - (0.11002313 * superOscSettings.detune))) / lastSampleRate));
        superOscSettings.phaseIncrements[2] = (double)(((newFrequency * (1 - (0.06288439 * superOscSettings.detune))) / lastSampleRate));
        superOscSettings.phaseIncrements[3] = (double)(((newFrequency * (1 - (0.01952356 * superOscSettings.detune))) / lastSampleRate));
        superOscSettings.phaseIncrements[4] = (double)(((newFrequency * (1 + (0.01991221 * superOscSettings.detune))) / lastSampleRate));
        superOscSettings.phaseIncrements[5] = (double)(((newFrequency * (1 + (0.06216538 * superOscSettings.detune))) / lastSampleRate));
        superOscSettings.phaseIncrements[6] = (double)(((newFrequency * (1 + (0.10745242 * superOscSettings.detune))) / lastSampleRate));
    }
    float polyFit(float x)
    {
        float y{ 0.0f };
        y = (10028.7312891634 * fast_power(x, 11)) - (50818.8652045924 * fast_power(x, 10)) + (111363.4808729368 * fast_power(x, 9)) -
            (138150.6761080548 * fast_power(x, 8)) + (106649.6679158292 * fast_power(x, 7)) - (53046.9642751875 * fast_power(x, 6)) + (17019.9518580080 * fast_power(x, 5)) -
            (3425.0836591318 * fast_power(x, 4)) + (404.2703938388 * fast_power(x, 3)) - (24.1878824391 * fast_power(x, 2)) + (0.6717417634 * x) + 0.0030115596;
        return y;
    }
    double sideVolume(float x)
    {
        float y{ 0.0f };
        y = -0.73764 * fast_power(x, 2) + 1.2841 * x + 0.044372;
        return y;
    }
    double mainVolume(float x)
    {
        double y{ 0.0f };
        y = -0.55366 * x + 0.99785;
        return y;
    }
    double fast_power(float base, int power) {
        double result = 1.0f;
        while (power > 0) {

            if (power % 2 == 1) { // Can also use (power & 1) to make code even faster
                result = std::fmod((result * base), MOD);
            }
            base = std::fmod((base * base), MOD);
            power = power / 2; // Can also use power >>= 1; to make code even faster
        }
        return result;
    }
    void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
    {
        if (v == state)
        {
            juce::Logger::outputDebugString("ValueTree property changed: " + id.toString());

            if (id == IDs::SuperWaveOsc)
            {
                if (id == IDs::SWdetuneS || id == IDs::SWvolumeS) {
                    setSuperOscillators(state[IDs::SWdetuneS], state[IDs::SWvolumeS]);
                }
                if (id == IDs::SWoctave) {
                    octavePitchShift = state.getProperty(IDs::SWoctave);
                    setPitchMod(octavePitchShift, coarseDetune);
                    getPhaseIncrement(state.getProperty(IDs::SWFrequency), state.getProperty(IDs::MidiNote));
                }
                if (id == IDs::SWdetune) {
                    coarseDetune = state.getProperty(IDs::SWdetune);
                    setPitchMod(octavePitchShift, coarseDetune);
                    getPhaseIncrement(state.getProperty(IDs::SWFrequency), state.getProperty(IDs::MidiNote));
                }
                if (id == IDs::SWtype) {
                    type = state.getProperty(IDs::SWtype);
                }
                if (id == IDs::SWgain) {
                    gain.setGainLinear(state[IDs::SWgain]);
                }
                if (id == IDs::SWFrequency)
                {
                    getPhaseIncrement(state[IDs::SWFrequency], state[IDs::MidiNote]);
                    juce::Logger::outputDebugString("Freq: " + std::to_string((float)state[IDs::SWFrequency]));
                }

            }
        }
    }
private:
    juce::ValueTree state;
    struct oscSuper
    {
        double detune{ 0.0f };
        double volume{ 0.0f };
        double volumeMain{ 0.0f };
        std::array<double, 7> phaseIncrements{0.0f};
        std::array<double, 7> phases;
        std::array<double, 7> lastOutputs{ 0.0f };
    };
    oscSuper superOscSettings;
    float type{ 0.0f };
    juce::dsp::Oscillator<float> fmOsc{ [](float x) {return std::sin(x); } };
    juce::dsp::Gain<float> gain;
    float lastFreq;
    double lastSampleRate{0.0f};
    float newFrequency{ 0.0f };
    float oldFrequency{ 0.0f };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
    int octavePitchShift{ 0 };
    float coarseDetune{ 0.0f };
    //float sampleRate;
};
