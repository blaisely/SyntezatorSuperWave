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
                       ),
                        state(*this,nullptr, juce::Identifier("VTS"),
                        createParameterLayout()),
                        DCOffset(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000,5.0)),
                        tree(createValueTree())
#endif
{
    mySynth.clearVoices();

    for (int i = 0; i < 6; ++i) {
        mySynth.addVoice(new SynthVoice(tree));
    }

    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
    mySynth.setNoteStealingEnabled(true);
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

    DCOffset.reset();

    DCOffset.prepare(spec);
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
    for (int i = 0; i < mySynth.getNumVoices(); i++) {

        if (myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))) {
            myVoice->update();
        }
    }
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
                myVoice->update();
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
    //return new SimpleSynthAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
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

    for (int i = 0; i < mySynth.getNumVoices(); i++) {

        if (myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))) {
            myVoice->reset();
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleSynthAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto logRange = makeLogarithmicRange(20.0f, 20480.0f);
    juce::NormalisableRange<float> attackRange{0.1f,150.0f,1.f,0.5};
    juce::NormalisableRange<float> decayRange{0.1f,150.0f,1.f,0.5};
    juce::NormalisableRange<float> sustainRange{0.0f,1.0f,0.01f};
    juce::NormalisableRange<float> releaseRange{0.1f,150.f,1.f,0.5};

    layout.add(std::make_unique<juce::AudioParameterFloat>("filterCutoff", "FilterCutOff",logRange ,20480.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterRes", "FilterRes",
        juce::NormalisableRange<float>(0.000,100.00,0.01,0.4),0.707));

    layout.add(std::make_unique<juce::AudioParameterInt>("filterDrive", "filterDrive", 1,20,1));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc1", "GainOsc1", 0.0f, 1.0f, 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("panOsc1", "Pan OSC 1", -1.0f, 1.0f, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc2", "GainOsc2", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("panOsc2", "Pan OSC 2", -1.0f, 1.0f, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterEnvelope", "Filter Envelope Amount", -100.0f, 100.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("octave_osc1", "Octave Osc1",
        juce::NormalisableRange<float>{-3,3,1},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("octave_osc2", "Octave Osc2",
        juce::NormalisableRange<float>{-3,3,1},-2));
    layout.add(std::make_unique<juce::AudioParameterFloat>("coarse_osc1", "coarse Osc1",
        juce::NormalisableRange<float>{-12,12,1},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("coarse_osc2", "coarse Osc2",
        juce::NormalisableRange<float>{-12,12,1},0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("attack", "Attack",attackRange,0.40f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay",decayRange,1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain",sustainRange , 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("release", "Release",releaseRange,1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("attackOsc2", "AttackEnv2(Osc2)",attackRange,0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decayOsc2", "DecayEnv2(Osc2)",decayRange,1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustainOsc2", "SustainEnv2(Osc2)",sustainRange , 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("releaseOsc2", "ReleaseEnv2(Osc2)",releaseRange,1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("lfodepth", "LFDepth",
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 1.f,0.2f}, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfofreq", "LFOFreq",
        juce::NormalisableRange<float>{ 0.01f, 20.0f, 0.01f,0.3f},0.1f));
    auto attributesLFOType = juce::AudioParameterChoiceAttributes().withLabel("LFO TYpe");
    layout.add(std::make_unique<juce::AudioParameterChoice>("lfoType", "LFO Type", juce::StringArray{ "Sine", "Square", "Saw"},
    0, attributesLFOType));
    auto attributesLFODestination = juce::AudioParameterChoiceAttributes().withLabel("LFO Destination");
    layout.add(std::make_unique<juce::AudioParameterChoice>("lfoDestination", "LFO Destination",
        juce::StringArray{ "Filter CutOff","Filter Resonance", "Gain OSC1", "Gain OSC2","Pan OSC1","Pan OSC2","Detune Volume","Detune Amount",
            "Drive Amount", "AMP Attack","AMP Decay","AMP Sustain","AMP Release","Env2 Attack","Env2 Decay","Env2 Sustain","Env2 Release"},
    0, attributesLFOType));

    layout.add(std::make_unique<juce::AudioParameterFloat>("detuneSuper", "DetuneAmount",
        juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.01f},0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("volumeSuper", "DetuneVolume",
        juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.01f}, 0.0f));

    auto attributesFilter = juce::AudioParameterChoiceAttributes().withLabel("selectedFilterType");

    layout.add(std::make_unique<juce::AudioParameterChoice>("filterType", "FilterType", juce::StringArray{ "LowPass", "HighPass", "BandPass" }
        , 0, attributesFilter));
    auto attributesOsc1Menu = juce::AudioParameterChoiceAttributes().withLabel("selectedOsc1Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("oscType_osc1", "OSC1 Type", juce::StringArray{ "Sine", "Saw", "Square", "Triangle" }
        , 0, attributesOsc1Menu));

    auto attributesOsc2Menu = juce::AudioParameterChoiceAttributes().withLabel("selectedOsc2Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("oscType_osc2", "OSC2 Type", juce::StringArray{ "Sine", "Saw", "Square", "Triangle" }
        , 0, attributesOsc2Menu));

    layout.add(std::make_unique<juce::AudioParameterBool>("filterbutton", "SVF Filter", 1));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfoReset", "lfoReset", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("commonEnvelope", "Shared Envelope", 1));

    return layout;
}
SimpleSynthAudioProcessor::chainSettings SimpleSynthAudioProcessor::getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    SimpleSynthAudioProcessor::chainSettings settings;

    settings.cutoff = apvts.getRawParameterValue("filterCutoff")->load();
    settings.resonance = apvts.getRawParameterValue("filterRes")->load();
    settings.filterDrive = apvts.getRawParameterValue("filterDrive")->load();
    settings.oscType_osc1 = apvts.getRawParameterValue("oscType_osc1")->load();
    settings.oscType_osc2 = apvts.getRawParameterValue("oscType_osc2")->load();
    settings.filterType = apvts.getRawParameterValue("filterType")->load();

    settings.attack = apvts.getRawParameterValue("attack")->load();
    settings.decay = apvts.getRawParameterValue("decay")->load();
    settings.sustain  = apvts.getRawParameterValue("sustain")->load();
    settings.release = apvts.getRawParameterValue("release")->load();

    settings.attackEnv2 = apvts.getRawParameterValue("attackOsc2")->load();
    settings.decayEnv2 = apvts.getRawParameterValue("decayOsc2")->load();
    settings.sustainEnv2  = apvts.getRawParameterValue("sustainOsc2")->load();
    settings.releaseEnv2 = apvts.getRawParameterValue("releaseOsc2")->load();

    settings.commonEnvelope = apvts.getRawParameterValue("commonEnvelope")->load();
    settings.envelopeAmount = apvts.getRawParameterValue("filterEnvelope")->load();

    settings.gain_osc1 = apvts.getRawParameterValue("gain_osc1")->load();
    settings.gain_osc2 = apvts.getRawParameterValue("gain_osc2")->load();

    settings.lfodepth = apvts.getRawParameterValue("lfodepth")->load();
    settings.lfofreq = apvts.getRawParameterValue("lfofreq")->load();
    settings.lfoReset = apvts.getRawParameterValue("lfoReset")->load();
    settings.lfoType = apvts.getRawParameterValue("lfoType")->load();

    settings.detune = apvts.getRawParameterValue("detuneSuper")->load();
    settings.volume = apvts.getRawParameterValue("volumeSuper")->load();
    settings.filterOn = apvts.getRawParameterValue("filterbutton")->load();

    settings.coarseosc1 = apvts.getRawParameterValue("coarse_osc1")->load();
    settings.coarseosc2 = apvts.getRawParameterValue("coarse_osc2")->load();
    settings.octaveosc2 = apvts.getRawParameterValue("octave_osc2")->load();
    settings.octaveosc1 = apvts.getRawParameterValue("octave_osc1")->load();

    settings.panOsc1 = apvts.getRawParameterValue("panOsc1")->load();
    settings.panOsc2 = apvts.getRawParameterValue("panOsc2")->load();

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
    tree.setProperty(IDs::ADSR2Attack, s.attackEnv2, nullptr);
    tree.setProperty(IDs::ADSR2Decay, s.decayEnv2, nullptr);
    tree.setProperty(IDs::ADSR2Sustain, s.sustainEnv2, nullptr);
    tree.setProperty(IDs::ADSR2Release, s.releaseEnv2, nullptr);
    tree.setProperty(IDs::SWgain, s.gain_osc1, nullptr);
    tree.setProperty(IDs::VAgain, s.gain_osc2, nullptr);
    tree.setProperty(IDs::LFODepth, s.lfodepth, nullptr);
    tree.setProperty(IDs::LFOFreq, s.lfofreq, nullptr);
    tree.setProperty(IDs::LFOType, s.lfoType, nullptr);
    tree.setProperty(IDs::SWdetuneS, s.detune, nullptr);
    tree.setProperty(IDs::SWvolumeS, s.volume, nullptr);
    tree.setProperty(IDs::SWdetune, s.coarseosc1, nullptr);
    tree.setProperty(IDs::VAdetune, s.coarseosc2, nullptr);
    tree.setProperty(IDs::SWoctave, s.octaveosc1, nullptr);
    tree.setProperty(IDs::VAoctave, s.octaveosc2, nullptr);
    tree.setProperty(IDs::SVFEnabled,s.filterOn,nullptr);
    tree.setProperty(IDs::FilterDrive,s.filterDrive,nullptr);
    tree.setProperty(IDs::LFOReset,s.lfoReset,nullptr);
    tree.setProperty(IDs::CommonEnvelope,s.commonEnvelope,nullptr);
    tree.setProperty(IDs::FilterEnvelopeAmount,s.envelopeAmount,nullptr);
    tree.setProperty(IDs::ReversedEnvelope,s.reversedEnvelope,nullptr);
    tree.setProperty(IDs::PanOsc1,s.panOsc1,nullptr);
    tree.setProperty(IDs::PanOsc2,s.panOsc2,nullptr);
}
