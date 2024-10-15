

#pragma once
#include "JuceHeader.h"
#include<random>
#include "SharedData.h"
#define MOD 1000000007
static juce::SmoothedValue<float> pulseWidth2 {0.5f};
class VAOsc:juce::ValueTree::Listener {
public:
    explicit VAOsc(juce::ValueTree v) :state(v), phase(0.0), phaseIncrement(0.0f)
    {
        state.addListener(this);
    }

    void prepareToPlay(const float& sampleRate,const int& samplesPerBlock,const int& outputChannels) {
        juce::dsp::ProcessSpec spec{};
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        gain.prepare(spec);
        lastSampleRate = sampleRate;
        for(auto &v:smoothedMod)
        {
            v.reset(sampleRate,0.001f);
        }
        pulseWidth.reset(sampleRate,0.001f);
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
        return (phase < (juce::MathConstants<float>::twoPi * pulseWidth2.getNextValue())) ? 1.0f : -1.0f;
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
            sample[i] = nextSample(phase,phaseIncrement,
                lastOutput);

            sample[i] = gain.processSample(sample[i]);
            sample[i] = sample[i]*0.5f;

        }
    }
    float getNextSample()
    {
        pulseWidth2.setTargetValue(std::clamp(pw+modValue[kPWM],0.1f,0.9f));
        smoothedMod[kOSC_TYPE].setTargetValue(std::clamp(type+modValue[kOSC_TYPE],0.f,3.f));
        updatePitch();
        float y=0;
        y = nextSample(phase,phaseIncrement,lastOutput);

        smoothedMod[kGAIN].setTargetValue(std::clamp(gainAmt+modValue[kGAIN],0.f,1.f));
        gain.setGainLinear(smoothedMod[kGAIN].getNextValue());
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

    float nextSample(float& phase,const float& phaseIncrement, float& lastOutput)  {
        const float t = phase / juce::MathConstants<float>::twoPi;
        float value = 0.0f;
        float value2 = 0.0f;
        float output = 0.0f;
        float type = smoothedMod[kOSC_TYPE].getNextValue();
        if(type>=0.0f && type<1.0f)
        {
            value = sine(phase);

            value2 = poly_saw(phase);
            value2 -= poly_blep(t, phaseIncrement);
            output = value*(1.f-type)+(value2*type);
        }
        if(type >= 1.f && type<2.f)
        {
            value = poly_saw(phase);
            value -= poly_blep(t, phaseIncrement);

            value2 = square(phase);
            value2 += poly_blep(t, phaseIncrement);
            value2 -= poly_blep(fmod(t + pulseWidth.getCurrentValue(), 1.0f), phaseIncrement);
            output = value*(2.f-type)+(value2*(type-1.f));
        }
        if(type>=2.f && type<=3.f)
        {
            value = square(phase);
            value += poly_blep(t, phaseIncrement);
            value -= poly_blep(fmod(t + pulseWidth.getCurrentValue(), 1.0f), phaseIncrement);

            value2 = triangle(phase);
            value2 += poly_blep(t, phaseIncrement);
            value2 -= poly_blep(fmod(t + 0.5f, 1.0f), phaseIncrement);
            value2 = phaseIncrement * value2 + (1.0f - phaseIncrement) * lastOutput;
            lastOutput = value2;
            output = value*(3.f-type)+(value2*(type-2.f));
        }
        if(type<0.f || type>3.f)
        {
            value = sine(phase);
            output = value;
        }
        phase += phaseIncrement;
        while (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        return output;
    }

    void setFrequency(const float& frequency,const int midiNote)
    {
        midiPitch = juce::MidiMessage::getMidiNoteInHertz(midiNote);
        updatePitch();
    }
    void updatePitch()
    {
        const float modPitch = std::pow(2.0f,(octave + detuneSemi+detuneFine)/12.f);
        float modulatedPitch = modValue[kPITCH]*midiPitch;

        const float freq =( midiPitch+modulatedPitch) * modPitch;
        this->oscFrequency = freq;
        phaseIncrement = (oscFrequency / lastSampleRate) * juce::MathConstants<float>::twoPi;
    }
    void setParameters()
    {
        type = state[IDs::VAtype];
        octave = static_cast<float>(state[IDs::VAoctave])*12;
        detuneSemi = state[IDs::VAdetune];
        detuneFine = static_cast<float>(state[IDs::VACoarse])/100.f;
        updatePitch();
        gainAmt = state[IDs::VAgain];
        gain.setGainLinear(gainAmt);
        pw = static_cast<float>(state[IDs::PulseWidthOSC2])/100.f;

    }
    float* getModPitch()
    {
        return &modValue[kPITCH];
    }
    float* getModGain()
    {
        return &modValue[kGAIN];
    }
    float* getModOscType()
    {
        return &modValue[kOSC_TYPE];
    }
    float* getModPWM()
    {
        return &modValue[kPWM];
    }
    void valueTreePropertyChanged(juce::ValueTree& v, const juce::Identifier& id) override
    {
    }
private:
    enum{kGAIN,kPITCH,kPWM,kOSC_TYPE,kNumDest};
    std::array<float,kNumDest> modValue{0.0f};
    std::array<juce::SmoothedValue<float>,kNumDest> smoothedMod{0.0f};
    juce::ValueTree state;
    float type{ 0.f };
    juce::dsp::Gain<float> gain;
    float lastOutput{};
    float phaseIncrement;
    float phase;
    float gainAmt{0.0f};
    float output{};
    float lastSampleRate{};
    float oscFrequency{ 0.0f };
    float octave{0};
    float detuneSemi{0};
    float detuneFine{0};
    float midiPitch{0.f};
    float pw{0.0f};
};

