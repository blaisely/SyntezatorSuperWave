

#pragma once
#include "JuceHeader.h"
#include<random>
#include "SharedData.h"
#define MOD 1000000007

class VAOsc:juce::ValueTree::Listener {
public:
    explicit VAOsc(juce::ValueTree v) :state(v), phase(0.0), phaseIncrement(0.0f)
    {
        state.addListener(this);
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
    void getNextBlock(const juce::dsp::AudioBlock<float>& block, const int channel)
    {
        const auto numSamples = block.getNumSamples();
        auto* sample = block.getChannelPointer(channel);

        for (size_t i = 0; i < numSamples; ++i)
        {
            sample[i] = nextSampleUniversal(phase,phaseIncrement,
                lastOutput);

            sample[i] = gain.processSample(sample[i]);
            sample[i] = sample[i]*0.5f;

        }
    }
    float getNextSample()
    {
        float y=0;
        y = nextSampleUniversal(phase,phaseIncrement,
               lastOutput);

        y = gain.processSample(y);
        y = y*0.5f;
        return y;
    }

    void resetOsc() {
        gain.reset();
        phase = 0.0f;
        phaseIncrement = 0.0f;
        lastOutput = 0.0f;

    }
    void setRandomPhase(const float& phase)
    {
        this->phase = phase;
    }

    float nextSampleUniversal(float& phase,const float& phaseIncrement, float& lastOutput) const {
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

        return static_cast<float>(value);
    }
    void prepareToPlay(const float& sampleRate,const int& samplesPerBlock,const int& outputChannels) {
        juce::dsp::ProcessSpec spec{};
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        gain.prepare(spec);
        lastSampleRate = sampleRate;
    }
    void setFrequency(const float& frequency,const int midiNote)
    {
        midiPitch = juce::MidiMessage::getMidiNoteInHertz(midiNote);
        updatePitch();
    }
    void updatePitch()
    {
        const float modPitch = std::pow(2.0f,(octave + detuneSemi)/12.f);
        const float freq = midiPitch * modPitch;
        this->oscFrequency = freq;
        phaseIncrement = (oscFrequency / lastSampleRate) * juce::MathConstants<float>::twoPi;
    }
    void setParameters()
    {
        type = state[IDs::VAtype];
        octave = static_cast<float>(state[IDs::VAoctave])*12;
        detuneSemi = state[IDs::VAdetune];
        updatePitch();
        gain.setGainLinear(state[IDs::VAgain]);

    }
    void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
    {

    }
private:
    juce::ValueTree state;
    int type{ 0 };
    juce::dsp::Gain<float> gain;
    float lastOutput{};
    float phaseIncrement;
    float phase;
    float output{};
    float lastSampleRate{};
    float oscFrequency{ 0.0f };
    float octave{0};
    float detuneSemi{0};
    float midiPitch{0.f};
};

