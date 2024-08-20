

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
            sample[i] = nextSampleUniversal(phase,phaseIncrement,
                lastOutput)*0.25;
            DBG("Second osc Out: "<<sample[i]);
            sample[i] = gain.processSample(sample[i]);

        }
    }

    void resetOsc() {
        gain.reset();
        phase = 0.0f;
        phaseIncrement = 0.0f;

    }
    void setRandomPhase(const double& phase)
    {
        this->phase = phase;
    }

    float nextSampleUniversal(double& phase,const double& phaseIncrement, double& lastOutput) const {
        const double t = phase / juce::MathConstants<float>::twoPi;
        double value = 0.0f;

        switch (static_cast<int>(state[IDs::VAtype])) {
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
    void prepareToPlay(const double& sampleRate,const int& samplesPerBlock,const int& outputChannels) {
        juce::dsp::ProcessSpec spec{};
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        gain.prepare(spec);

        lastSampleRate = sampleRate;
    }
    void setFrequency(const double& frequency,const int midiNote)
    {
        midiPitch = juce::MidiMessage::getMidiNoteInHertz(midiNote);
       updatePitch();
    }
    void updatePitch()
    {
        const double modPitch = std::pow(2.0f,(octave + detuneSemi)/12.f);
        const double freq = midiPitch * modPitch;
        this->oscFrequency = freq;
        const double phaseInc = freq / lastSampleRate;
        phaseIncrement = phaseInc;

    }
    void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
    {
	    if(v==state)
	    {
               if(id==IDs::VAFrequency)
               {
               }
                if(id==IDs::VAtype)
                {
                    type = state[IDs::VAtype];
                }
                if(id==IDs::VAoctave || id==IDs::VAdetune)
                {
                    octave = static_cast<float>(state[IDs::VAoctave])*12;
                    detuneSemi = state[IDs::VAdetune];
                    updatePitch();
                }
                if(id==IDs::VAgain)
                {
                    gain.setGainLinear(state[IDs::VAgain]);
                }

	    }
    }
private:
    juce::ValueTree state;
    float type{ 0.0f };
    juce::dsp::Gain<float> gain;
    double lastOutput{};
    double phaseIncrement;
    double phase;
    double output{};
    double lastSampleRate{};
    double oscFrequency{ 0.0f };
    float octave{0};
    float detuneSemi{0};
    double midiPitch{0.f};
    //float sampleRate;
};

