/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

template<typename FloatType>
static inline juce::NormalisableRange<FloatType> makeLogarithmicRange(FloatType min, FloatType max)
{
    return juce::NormalisableRange<FloatType>
        (
            min, max,
            [](FloatType start, FloatType end, FloatType normalised)
            {
                return start + (std::pow(FloatType(2), normalised * FloatType(10)) - FloatType(1)) * (end - start) / FloatType(1023);
            },
            [](FloatType start, FloatType end, FloatType value)
            {
                return (std::log(((value - start) * FloatType(1023) / (end - start)) + FloatType(1)) / std::log(FloatType(2))) / FloatType(10);
            }
                );
}
//==============================================================================
SimpleSynthAudioProcessor::SimpleSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), s1(0), s2(0),state(*this,nullptr, juce::Identifier("VTS"), createParameterLayout()), lowPassFilter(juce::dsp::IIR::Coefficients<float>::makeLowPass(48000,20000.0f, 0.1f)),DCOffset(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000,5.0)),tree(createValueTree())
#endif
{
    mySynth.clearVoices();

    for (int i = 0; i < 4; ++i) {
        mySynth.addVoice(new SynthVoice(tree));
    }

    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
}

SimpleSynthAudioProcessor::~SimpleSynthAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    lowPassFilter.reset();
    DCOffset.reset();
    lowPassFilter.prepare(spec);
    DCOffset.prepare(spec);
    //TPTFfilter.setType(filterType);
    lastSampleRate = sampleRate;
    mySynth.setCurrentPlaybackSampleRate(lastSampleRate);

    for (int i = 0; i < mySynth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
}

void SimpleSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif
void SimpleSynthAudioProcessor::updateFilter() {
    auto chainSettings = getChainSettings(state);
    float freq = chainSettings.cutoff;
    float res = chainSettings.resonance;
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), freq, res);
    *DCOffset.state = *juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(), 20.0f);
}
void SimpleSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto chainSettings = getChainSettings(state);
    
    syncStates(tree,chainSettings);
   
    for (int i = 0; i < mySynth.getNumVoices(); i++) {

        if (myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))) {
           myVoice->setEnvelope(&chainSettings.attack, &chainSettings.decay, &chainSettings.sustain, &chainSettings.release);
           //myVoice->checkForFilter(&chainSettings.filterOn);
        }
    }
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {

        buffer.clear(i, 0, buffer.getNumSamples());
    }
    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);
    updateFilter();
    DCOffset.process(context);
}

//==============================================================================
bool SimpleSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleSynthAudioProcessor::createEditor()
{
    return new SimpleSynthAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
}

void SimpleSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSynthAudioProcessor();
}
void SimpleSynthAudioProcessor::reset() {

    

}

void SimpleSynthAudioProcessor::IIRfilter(float frequency, float sampleRate, float* samples,float numSamples) {

    const float g = tanf(juce::float_Pi * frequency / sampleRate);
    const float h = 1.f / (1 + g / Q + g * g);

    for (int i = 0; i < numSamples; i++) {
        const float in = samples[i];
        const float yH = h * (in - (1.f / Q + g) * s1 - s2);
        const float yB = g * yH + s1;
        s1 = g * yH + yB;
        const float yL = g * yB + s2;
        s2 = g * yB + yL;
        samples[i] = yL;

    }

}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleSynthAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto logRange = makeLogarithmicRange(20.0f, 20000.0f);
    auto logRangeAttack = makeLogarithmicRange(0.001f, 10.0f);
    auto logRangeDecay = makeLogarithmicRange(0.001f, 10.0f);
    auto logRangeSustain = makeLogarithmicRange(0.001f, 1.0f);
    auto logRangeRelease = makeLogarithmicRange(0.001f, 10.0f);

    layout.add(std::make_unique<juce::AudioParameterFloat>("filterCutoff", "FilterCutOff",logRange ,20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterRes", "FilterRes", 0.707f, 10.0f, 0.707f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc1", "Gain_osc1", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc2", "Gain_osc2", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterInt>("octave_osc1", "Octave_osc1", -3, 3, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>("octave_osc2", "Octave_osc2", -3, 3, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>("coarse_osc1", "coarse_osc1", -12, 12, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("coarse_osc2", "coarse_osc2", -12, 12, 0.));

    layout.add(std::make_unique<juce::AudioParameterFloat>("attack", "Attack",logRangeAttack,0.001f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay",logRangeDecay,0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain",logRangeSustain , 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("release", "Release",logRangeRelease,0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("attack_osc2", "Attack_osc2", 0.1f, 10.f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay_osc2", "Decay_osc", 0.1f, 15.f, 6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustain_osc2", "Sustain_osc2", 0.1f, 1.f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("release_osc2", "Release_osc2", 0.1f, 10.f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("lfodepth", "LFDepth", juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.1f,0.3f}, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfofreq", "LFOFreq", juce::NormalisableRange<float>{ 0.0f, 30.0f, 0.1f,0.3f},0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("detuneSuper", "DetuneSuper", juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.01f},0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("volumeSuper", "volumeSuper", juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.01f}, 0.0f));
    

    auto attributesFilter = juce::AudioParameterChoiceAttributes().withLabel("selectedFilterType");

    layout.add(std::make_unique<juce::AudioParameterChoice>("filterType", "FilterType", juce::StringArray{ "LowPass", "HighPass", "BandPass" }, 0, attributesFilter));

    auto attributesOsc1Menu = juce::AudioParameterChoiceAttributes().withLabel("selectedOsc1Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("oscType_osc1", "OscType_osc1", juce::StringArray{ "Sine", "Saw", "Square", "Triangle" }, 0, attributesOsc1Menu));

    auto attributesOsc2Menu = juce::AudioParameterChoiceAttributes().withLabel("selectedOsc2Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("oscType_osc2", "OscType_osc2", juce::StringArray{ "Sine", "Saw", "Square", "Triangle" }, 0, attributesOsc2Menu));

    layout.add(std::make_unique<juce::AudioParameterBool>("filterbutton", "FilterButton", 0));

    return layout;
}
SimpleSynthAudioProcessor::chainSettings SimpleSynthAudioProcessor::getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    SimpleSynthAudioProcessor::chainSettings settings;

    settings.cutoff = apvts.getRawParameterValue("filterCutoff")->load();
    settings.resonance = apvts.getRawParameterValue("filterRes")->load();
    settings.oscType_osc1 = apvts.getRawParameterValue("oscType_osc1")->load();
    settings.oscType_osc2 = apvts.getRawParameterValue("oscType_osc2")->load();
    settings.filterType = apvts.getRawParameterValue("filterType")->load();

    settings.attack = apvts.getRawParameterValue("attack")->load();
    settings.decay = apvts.getRawParameterValue("decay")->load();
    settings.sustain  = apvts.getRawParameterValue("sustain")->load();
    settings.release = apvts.getRawParameterValue("release")->load();

    settings.gain_osc1 = apvts.getRawParameterValue("gain_osc1")->load();
    settings.gain_osc2 = apvts.getRawParameterValue("gain_osc2")->load();

    settings.lfodepth = apvts.getRawParameterValue("lfodepth")->load();
    settings.lfofreq = apvts.getRawParameterValue("lfofreq")->load();

    settings.detune = apvts.getRawParameterValue("detuneSuper")->load();
    settings.volume = apvts.getRawParameterValue("volumeSuper")->load();

    settings.filterOn = apvts.getRawParameterValue("filterbutton")->load();
    settings.coarseosc1 = apvts.getRawParameterValue("coarse_osc1")->load();
    settings.coarseosc2 = apvts.getRawParameterValue("coarse_osc2")->load();
    settings.octaveosc2 = apvts.getRawParameterValue("octave_osc2")->load();
    settings.octaveosc1 = apvts.getRawParameterValue("octave_osc1")->load();
   

    return settings;
}

juce::ValueTree SimpleSynthAudioProcessor::createValueTree()
{
    juce::ValueTree v(IDs::Parameters);
    juce::ValueTree Osc(IDs::Oscillator);
    juce::ValueTree Filter(IDs::Filter);
    juce::ValueTree Lfo(IDs::LFO);
    juce::ValueTree SWosc(IDs::SuperWaveOsc);
    juce::ValueTree VAosc(IDs::VAOsc);
    juce::ValueTree ADSR1(IDs::ADSR1);
    juce::ValueTree ADSR2(IDs::ADSR2);

    v.addChild(Osc, -1, nullptr);
    v.addChild(Filter, -1, nullptr);
    v.addChild(Lfo, -1, nullptr);
    v.addChild(ADSR1, -1, nullptr);
    v.addChild(ADSR2, -1, nullptr);
    
    Osc.addChild(SWosc, -1, nullptr);
    Osc.addChild(VAosc, -1, nullptr);

    SWosc.setProperty(IDs::SWdetune, 0.0f, nullptr);
    SWosc.setProperty(IDs::SWdetuneS, 0.0f, nullptr);
    SWosc.setProperty(IDs::SWgain, 0.0f, nullptr);
    SWosc.setProperty(IDs::SWoctave, 0.0f, nullptr);
    SWosc.setProperty(IDs::SWtype, 0.0f, nullptr);
    SWosc.setProperty(IDs::SWvolumeS, 0.0f, nullptr);

    VAosc.setProperty(IDs::VAdetune, 0.0, nullptr);
    VAosc.setProperty(IDs::VAgain, 0.0, nullptr);
    VAosc.setProperty(IDs::VAtype, 0.0, nullptr);
    VAosc.setProperty(IDs::VAoctave, 0.0, nullptr);

    Filter.setProperty(IDs::Cutoff, 0.0f, nullptr);
    Filter.setProperty(IDs::Resonance, 0.1f, nullptr);

    Lfo.setProperty(IDs::LFODepth, 0.0f, nullptr);
    Lfo.setProperty(IDs::LFOFreq, 0.0f, nullptr);
    Lfo.setProperty(IDs::LFOMod, 0.0f, nullptr);
    Lfo.setProperty(IDs::LFOType, 0.0f, nullptr);

    ADSR1.setProperty(IDs::ADSR1Attack, 0.0f, nullptr);
    ADSR1.setProperty(IDs::ADSR1Decay, 0.0f, nullptr);
    ADSR1.setProperty(IDs::ADSR1Sustain, 0.0f, nullptr);
    ADSR1.setProperty(IDs::ADSR1Release, 0.0f, nullptr);

    ADSR2.setProperty(IDs::ADSR2Attack, 0.0f, nullptr);
    ADSR2.setProperty(IDs::ADSR2Decay, 0.0f, nullptr);
    ADSR2.setProperty(IDs::ADSR2Sustain, 0.0f, nullptr);
    ADSR2.setProperty(IDs::ADSR2Release, 0.0f, nullptr);
    ADSR2.setProperty(IDs::ADSR2Mod, 0.0f, nullptr);
    return v;

}

void SimpleSynthAudioProcessor::syncStates(juce::ValueTree& tree,chainSettings& s)
{
    tree.setProperty(IDs::Cutoff, s.cutoff, nullptr);
    tree.setProperty(IDs::Resonance, s.resonance, nullptr);
    tree.setProperty(IDs::SWtype, s.oscType_osc1, nullptr);
    tree.setProperty(IDs::VAtype, s.oscType_osc2, nullptr);
    tree.setProperty(IDs::FilterT, s.filterType, nullptr);
    tree.setProperty(IDs::ADSR1Attack, s.attack, nullptr);
    tree.setProperty(IDs::ADSR1Decay, s.decay, nullptr);
    tree.setProperty(IDs::ADSR1Sustain, s.sustain, nullptr);
    tree.setProperty(IDs::ADSR1Release, s.release, nullptr);
    tree.setProperty(IDs::SWgain, s.gain_osc1, nullptr);
    tree.setProperty(IDs::VAgain, s.gain_osc2, nullptr);
    tree.setProperty(IDs::LFODepth, s.lfodepth, nullptr);
    tree.setProperty(IDs::LFOFreq, s.lfofreq, nullptr);
    tree.setProperty(IDs::SWdetuneS, s.detune, nullptr);
    tree.setProperty(IDs::SWvolumeS, s.volume, nullptr);
    tree.setProperty(IDs::SWdetune, s.coarseosc1, nullptr);
    tree.setProperty(IDs::VAdetune, s.coarseosc2, nullptr);
    tree.setProperty(IDs::SWoctave, s.octaveosc1, nullptr);
    tree.setProperty(IDs::VAoctave, s.octaveosc2, nullptr);

}
