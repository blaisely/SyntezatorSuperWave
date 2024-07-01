

#pragma once
#include "JuceHeader.h"
#include<random>
#include "SharedData.h"
#define MOD 1000000007

class VAOsc:juce::ValueTree::Listener {
public:
    VAOsc(juce::ValueTree v) :state(v), phase(0.0), phaseIncrement(0.0f), keyTrack(juce::dsp::IIR::Coefficients<float>::makeHighPass(48000, 0.1f))
    {
        state.addListener(this);
    }
    double OscType(int oscillatorType, float phase) {

        switch (oscillatorType) {
        case 0:
            return sine(phase);
            break;
        case 1:
            return  poly_saw(phase);
            break;
        case 2:
            return square(phase);
            break;
        case 3:
            return triangle(phase);
            break;
        default:
            return  sine(phase);
            break;

        }
    }

    void getType(float oscType) {

        type = oscType;
    }

    double poly_blep(double t, float phaseIncrement)
    {
        // 0 <= t < 1
        double dt = phaseIncrement / (juce::MathConstants<float>::twoPi);



        if (t < dt)
        {
            t /= dt;
            // 2 * (t - t^2/2 - 0.5)
            return t + t - t * t - 1.;
        }

        // -1 < t < 0
        else if (t > 1. - dt)
        {
            t = (t - 1.) / dt;
            // 2 * (t^2/2 + t + 0.5)
            return t * t + t + t + 1.;
        }

        // 0 otherwise
        else
        {
            return 0.;
        }
    }

    double simpleSaw(double frequency) {
        //Sawtooth generator. This is like a phasor but goes between -1 and 1
        //output = 2. * phase - 1.;
       // phase += 0.5;
        output = phase;
        if (phase >= 1.0) phase -= 1.0;
        phase += (1. / (48000.0 / (frequency))) * 2.0;
        return output; //- poly_blep(phase, frequency / 48000.0f);
    }
    //----------------------------------------------------------------
    double poly_saw(float phase)
    {
        double value = (2 * phase / (2 * juce::double_Pi)) - 1.0;
        return value;

    }
    double sine(float phase) {
        double value = sin(phase);
        return value;
    }
    double square(float phase) {
        double value;
        if (phase < juce::double_Pi) {
            value = 1.0;
        }
        else {
            value = -1.0;
        }
        return value;
    }
    double triangle(float phase) {
        double value;
        value = -1.0 + (2.0 * phase / (2 * juce::double_Pi));
        value = 2.0 * (fabs(value) - 0.5);
        return value;
    }

    void getNextBlock(juce::dsp::AudioBlock<float>& block, const int channel)
    {
        auto numSamples = block.getNumSamples();
        auto numChannels = block.getNumChannels();
        auto* sample = block.getChannelPointer(channel);
        auto* sample2 = block.getChannelPointer(1);
        for (size_t i = 0; i < numSamples; i++)
        {
            sample[i] = gain.processSample(nextSample());
        }
    }
    void resetOsc() {
        phase = 0.0f;
        gain.reset();
        keyTrack.reset();
    }
    //----------------------------------------------------------------
    float randomPhase()
    {
        std::random_device rd;  // Random number from hardware
        std::mt19937 gen(rd()); // Seed the generator
        std::uniform_real_distribution<> distr(0.0, 1.0); // Define the range
        float randomValue = distr(gen);
        return randomValue;
        //std::cout << "Random float value between 0 and 1: " << randomValue << std::endl;

    }
    void setRandomPhase(float phase)
    {
        this->phase = phase;
    }
    float nextSample() {

        double value = 0;
        double t = phase / (2 * juce::double_Pi);

        if (static_cast<int>(type) == 0.0f) {
            value = OscType(0, phase);
        }
        else if (static_cast<int>(type) == 1.0f) {
            value = OscType(1, phase);
            value -= poly_blep(t, phaseIncrement);
        }
        else {
            value = OscType(2, phase);
            value += poly_blep(t, phaseIncrement);
            value -= poly_blep(fmod(t + 0.5, 1.0), phaseIncrement);
            if (static_cast<int>(type) == 3) {
                //Leaky integrator: y[n] = A * x[n] + (1 - A) * y[n-1]
                value = phaseIncrement * value + (1 - phaseIncrement) * lastOutput;
                lastOutput = value;
            }
        }

        phase += phaseIncrement;
        while (phase >= (2 * juce::double_Pi)) {
            phase -= (2 * juce::double_Pi);
        }
        return value;


    }

//----------------------------------------
    float nextKeyTrackedsample()
    {
        phase += phaseIncrement;
        while (phase >= (2 * juce::double_Pi)) {
            phase -= (2 * juce::double_Pi);
        }
        return OscType(type, phase);
    }
    void setKeyTrackedFreq()
    {
        if (!juce::approximatelyEqual(newFrequency, oldFrequency)) {
            *keyTrack.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(48000, newFrequency, 0.1f);
        }

    }
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels) {
        lastSampleRate = sampleRate;
        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.sampleRate = sampleRate;
        spec.numChannels = outputChannels;
        //fmOsc.prepare(spec);
        gain.prepare(spec);
        keyTrack.prepare(spec);
    }

    void setPitchMod( float oct, float detune)
    {
        currentPitchShift = fast_power(2.0, (oct * 12 + detune) / 12);
    }

    void getPhaseIncrement(const double frequency) {
        oldFrequency = newFrequency;
        this->newFrequency = frequency*currentPitchShift;
        phaseIncrement = newFrequency / lastSampleRate;
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
	    if(v==state)
	    {
		    if(id==IDs::VAOsc)
		    {
               if(id==IDs::VAFrequency)
               {
                   getPhaseIncrement(state[IDs::VAFrequency]);
               }
                if(id==IDs::VAtype)
                {
                    type = state[IDs::VAtype];
                }
                if(id==IDs::VAoctave || id==IDs::VAdetune)
                {
                    setPitchMod(state[IDs::VAoctave], state[IDs::VAdetune]);
                }
                if(id==IDs::VAgain)
                {
                    gain.setGainLinear(state[IDs::VAGainOvr]);
                }
		    }
	    }
    }
private:
    juce::ValueTree state;
    float type{ 0.0f };
    juce::dsp::Gain<float> gain;
    float lastFreq;
    double lastOutput;
    double phaseIncrement;
    double phase;
    float amplitude;
    double output;
    double lastSampleRate;
    float newFrequency{ 0.0f };
    float oldFrequency{ 0.0f };
    float octave{ 0.0f };
    float coarse{ 0.0f };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>  keyTrack;
    float currentPitchShift{ 0.0f };
    //float sampleRate;
};

