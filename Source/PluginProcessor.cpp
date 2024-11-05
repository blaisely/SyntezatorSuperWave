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
                        DCOffset(juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(48000,20.0)),
                        tree(createValueTree())
#endif
{
    mySynth.clearVoices();

    for (int i = 0; i < 6; ++i) {
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
    DCOffset.reset();
    DCOffset.prepare(spec);
    pluginGain.prepare(spec);
    lastSampleRate = sampleRate;
    gainAmt.reset(lastSampleRate,0.1);
    updateFilter();
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
   reset();
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
    *DCOffset.state = *juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(), 20.0f);
}
void SimpleSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto chainSettings = getChainSettings(state);
    
    syncStates(tree,chainSettings);
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {

        buffer.clear(i, 0, buffer.getNumSamples());
    }
    for (int i = 0; i < mySynth.getNumVoices(); i++) {

        if (myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))) {
                myVoice->update();
        }
    }

    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);

    DCOffset.process(context);
    gainAmt.setTargetValue(static_cast<float>(tree[IDs::GainOvr]));

    //Process every sample by gain value
        for(auto sample=0;sample<buffer.getNumSamples();sample++)
        {
            pluginGain.setGainLinear(gainAmt.getNextValue());
            auto g = pluginGain.processSample(buffer.getSample(0,sample));
            auto g2 = pluginGain.processSample(buffer.getSample(1,sample));
            buffer.setSample(0,sample,g);
            buffer.setSample(1,sample,g2);
        }

}

//==============================================================================
bool SimpleSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleSynthAudioProcessor::createEditor()
{
    return new SimpleSynthAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
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
    pluginGain.reset();
    DCOffset.reset();
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleSynthAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto logRange = makeLogarithmicRange(20.0f, 20480.0f);
    juce::NormalisableRange<float> attackRange{0.0f,100.0f,1.f,0.7};
    juce::NormalisableRange<float> decayRange{0.0f,100.0f,1.f,0.7};
    juce::NormalisableRange<float> sustainRange{0.0f,100.0f,1.f};
    juce::NormalisableRange<float> releaseRange{0.0f,100.f,1.f,0.7};
    layout.add(std::make_unique<juce::AudioParameterFloat>("gainOVR","Gain",0.f,1.f,0.1f));
    //Filter Params
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterCutoff", "FilterCutOff",logRange ,20480.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterRes", "FilterRes",
        juce::NormalisableRange<float>(0.000,100.00,0.01,0.4),0.707,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    auto attributesFilter = juce::AudioParameterChoiceAttributes().withLabel("selectedFilterType");
    layout.add(std::make_unique<juce::AudioParameterChoice>("filterType", "FilterType", juce::StringArray{ "LowPass", "HighPass", "BandPass" }
        , 0, attributesFilter));
    layout.add(std::make_unique<juce::AudioParameterBool>("filterKeytrackEnable", "Filter Keytrack Enable",0));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "filterKeytrackOffset", "Filter Keytrack Offset",-48,48,0));
    layout.add(std::make_unique<juce::AudioParameterInt>("filterDrive", "filterDrive", 1,100,1,
        juce::AudioParameterIntAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterVelocity","Filter Velocity Sensitivity",juce::NormalisableRange<float>{0,100,1},0,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    //Gain Params
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc1", "GainOsc1", 0.0f, 1.0f, 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("panOsc1", "Pan OSC 1", -1.0f, 1.0f, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc2", "GainOsc2", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("panOsc2", "Pan OSC 2", -1.0f, 1.0f, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterEnvelope", "Filter Envelope Amount", -100.0f, 100.0f, 0.0f));
    //Tuning Params
    layout.add(std::make_unique<juce::AudioParameterFloat>("octave_osc1", "Octave Osc1",
        juce::NormalisableRange<float>{-3,3,1},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("octave_osc2", "Octave Osc2",
        juce::NormalisableRange<float>{-3,3,1},-2));
    layout.add(std::make_unique<juce::AudioParameterFloat>("coarse_osc1", "coarse Osc1",
        juce::NormalisableRange<float>{-12,12,1},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("coarse_osc2", "coarse Osc2",
        juce::NormalisableRange<float>{-12,12,1},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("detune_osc1", "Fine Detune Osc1",
        juce::NormalisableRange<float>{-100,100,1},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("detune_osc2", "Fine Detune Osc2",
        juce::NormalisableRange<float>{-100,100,1},0));

    //ADSR Params
    layout.add(std::make_unique<juce::AudioParameterFloat>("attack", "Attack",attackRange,
        0.40f,juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay",decayRange,1.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain",sustainRange , 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("release", "Release",releaseRange,1.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>("attackOsc2", "AttackEnv2(Osc2)",attackRange,0.4f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decayOsc2", "DecayEnv2(Osc2)",decayRange,1.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustainOsc2", "SustainEnv2(Osc2)",sustainRange , 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("releaseOsc2", "ReleaseEnv2(Osc2)",releaseRange,1.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>("attackOsc3", "AttackEnv3",attackRange,0.4f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decayOsc3", "DecayEnv3",decayRange,1.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustainOsc3", "SustainEnv3",sustainRange , 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("releaseOsc3", "ReleaseEnv3",releaseRange,1.f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    //LFO Params
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfodepth", "LF0 Depth",
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 1.f}, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfofreq", "LFO Freq",
        juce::NormalisableRange<float>{ 0.01f, 20.0f, 0.01f,0.3f},0.1f));
    auto attributesLFOType = juce::AudioParameterChoiceAttributes().withLabel("LFO Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("lfoType", "LFO Type", juce::StringArray{ "Sine", "Square", "Saw"
    ,"Sample&Hold"},
    0, attributesLFOType));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfo2depth", "LFO2 Depth",
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 1.f,0.2f}, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfo2freq", "LFO2 Freq",
        juce::NormalisableRange<float>{ 0.01f, 20.0f, 0.01f,0.3f},0.1f));
    auto attributesLFO2Type = juce::AudioParameterChoiceAttributes().withLabel("LFO2 Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("lfo2Type", "LFO2 Type", juce::StringArray{ "Sine", "Square", "Saw","Sample&Hold"},
    0, attributesLFO2Type));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfo3depth", "LFO3 Depth",
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 1.f}, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("lfo3freq", "LFO3 Freq",
        juce::NormalisableRange<float>{ 0.01f, 20.0f, 0.01f,0.3f},0.1f));
    auto attributesLFO3Type = juce::AudioParameterChoiceAttributes().withLabel("LFO3 Type");
    layout.add(std::make_unique<juce::AudioParameterChoice>("lfo3Type", "LFO3 Type", juce::StringArray{ "Sine", "Square", "Saw","Sample&Hold"},
    0, attributesLFO3Type));
    //Mod Matrix Params
    auto attributesModDestination1 = juce::AudioParameterChoiceAttributes().withLabel("MOD Destination 1");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modDestination1", "Mod Destination 1",
    juce::StringArray{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO1 Frequency","LFO2 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination1));
    auto attributesModSource1 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 1");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource1", "Mod Source 1",
        juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    0, attributesModDestination1));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity1","Mod Intensity 1",
        juce::NormalisableRange<float>(0.f,100.f,1.f),0.f));

    auto attributesModDestination2 = juce::AudioParameterChoiceAttributes().withLabel("MOD Destination 2");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modDestination2", "Mod Destination 2",
    juce::StringArray{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO1 Frequency","LFO2 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination2));
    auto attributesModSource2 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 2");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource2", "Mod Source 2",
    juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    1, attributesModDestination2));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity2","Mod Intensity 2",
        juce::NormalisableRange<float>(0.f,100.f,1.f),0.f));

    auto attributesModDestination3 = juce::AudioParameterChoiceAttributes().withLabel("MOD Destination 3");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modDestination3", "Mod Destination 3",
    juce::StringArray{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO1 Frequency","LFO2 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination3));
    auto attributesModSource3 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 3");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource3", "Mod Source 3",
    juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    2, attributesModDestination3));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity3","Mod Intensity 3",
        juce::NormalisableRange<float>(0.f,100.f,1.f),0.f));

    auto attributesModDestination4 = juce::AudioParameterChoiceAttributes().withLabel("MOD Destination 4");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modDestination4", "Mod Destination 4",
    juce::StringArray{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO1 Frequency","LFO2 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination4));
    auto attributesModSource4 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 4");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource4", "Mod Source 4",
    juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    2, attributesModDestination4));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity4","Mod Intensity 4",
        juce::NormalisableRange<float>(0.f,100.f,1.f),0.f));
    //SuperSaw params
    layout.add(std::make_unique<juce::AudioParameterFloat>("detuneSuper", "Detune Amount",
        juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.01f},0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("volumeSuper", "Detune Mix",
        juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.01f}, 0.0f));
    //OSC params
    layout.add(std::make_unique<juce::AudioParameterFloat>("oscType_osc1","OSC1 Type",
        juce::NormalisableRange<float>{0.0f,3.0f,0.1f},0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("pulseWidthOsc1","Pulse Width OSC1",
        juce::NormalisableRange<float>{10,90,1},50,juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("oscType_osc2","OSC2 Type",
        juce::NormalisableRange<float>{0.0f,3.0f,0.1f},0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("pulseWidthOsc2","Pulse Width OSC2",
        juce::NormalisableRange<float>{10,90,1},50,juce::AudioParameterFloatAttributes().withLabel("%")));
    //Additional Params
    layout.add(std::make_unique<juce::AudioParameterBool>("filterbutton", "SVF Filter ON", 1));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfoReset", "LFO Reset", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("commonEnvelope", "Shared Envelope for OSCs", 1));
    layout.add(std::make_unique<juce::AudioParameterBool>("loopEnvelope", "Loop Mod Envelope1", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("loopEnvelope2", "Loop Mod Envelope2", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfo1Unipolar", "LFO1 Unipolar", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfo2Unipolar", "LFO2 Unipolar", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfo3Unipolar", "LFO3 Unipolar", 0));


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
    settings.attackEnv3 = apvts.getRawParameterValue("attackOsc3")->load();
    settings.decayEnv3 = apvts.getRawParameterValue("decayOsc3")->load();
    settings.sustainEnv3  = apvts.getRawParameterValue("sustainOsc3")->load();
    settings.releaseEnv3 = apvts.getRawParameterValue("releaseOsc3")->load();
    settings.commonEnvelope = apvts.getRawParameterValue("commonEnvelope")->load();
    settings.envelopeAmount = apvts.getRawParameterValue("filterEnvelope")->load();
    settings.gain_osc1 = apvts.getRawParameterValue("gain_osc1")->load();
    settings.gain_osc2 = apvts.getRawParameterValue("gain_osc2")->load();
    settings.lfodepth = apvts.getRawParameterValue("lfodepth")->load();
    settings.lfo2depth = apvts.getRawParameterValue("lfo2depth")->load();
    settings.lfo3depth = apvts.getRawParameterValue("lfo3depth")->load();
    settings.lfofreq = apvts.getRawParameterValue("lfofreq")->load();
    settings.lfo2freq = apvts.getRawParameterValue("lfo2freq")->load();
    settings.lfo3freq = apvts.getRawParameterValue("lfo3freq")->load();
    settings.lfoReset = apvts.getRawParameterValue("lfoReset")->load();
    settings.lfoType = apvts.getRawParameterValue("lfoType")->load();
    settings.lfo2Type = apvts.getRawParameterValue("lfo2Type")->load();
    settings.lfo3Type = apvts.getRawParameterValue("lfo3Type")->load();
    settings.detune = apvts.getRawParameterValue("detuneSuper")->load();
    settings.volume = apvts.getRawParameterValue("volumeSuper")->load();
    settings.filterOn = apvts.getRawParameterValue("filterbutton")->load();
    settings.coarseosc1 = apvts.getRawParameterValue("coarse_osc1")->load();
    settings.coarseosc2 = apvts.getRawParameterValue("coarse_osc2")->load();
    settings.octaveosc2 = apvts.getRawParameterValue("octave_osc2")->load();
    settings.octaveosc1 = apvts.getRawParameterValue("octave_osc1")->load();
    settings.detuneosc1 = apvts.getRawParameterValue("detune_osc1")->load();
    settings.detuneosc2 = apvts.getRawParameterValue("detune_osc2")->load();
    settings.panOsc1 = apvts.getRawParameterValue("panOsc1")->load();
    settings.panOsc2 = apvts.getRawParameterValue("panOsc2")->load();
    settings.gainOVR = apvts.getRawParameterValue("gainOVR")->load();
    settings.modDestination1 = apvts.getRawParameterValue("modDestination1")->load();
    settings.modSource1 = apvts.getRawParameterValue("modSource1")->load();
    settings.modIntensity1 = apvts.getRawParameterValue("modIntensity1")->load();
    settings.modDestination2 = apvts.getRawParameterValue("modDestination2")->load();
    settings.modSource2 = apvts.getRawParameterValue("modSource2")->load();
    settings.modIntensity2 = apvts.getRawParameterValue("modIntensity2")->load();
    settings.modDestination3 = apvts.getRawParameterValue("modDestination3")->load();
    settings.modSource3 = apvts.getRawParameterValue("modSource3")->load();
    settings.modIntensity3 = apvts.getRawParameterValue("modIntensity3")->load();
    settings.modDestination4 = apvts.getRawParameterValue("modDestination4")->load();
    settings.modSource4 = apvts.getRawParameterValue("modSource4")->load();
    settings.modIntensity4 = apvts.getRawParameterValue("modIntensity4")->load();
    settings.loopModEnvelope = apvts.getRawParameterValue("loopEnvelope")->load();
    settings.loopModEnvelope2 = apvts.getRawParameterValue("loopEnvelope2")->load();
    settings.pulseWidthOsc1 = apvts.getRawParameterValue("pulseWidthOsc1")->load();
    settings.pulseWidthOsc2 = apvts.getRawParameterValue("pulseWidthOsc2")->load();
    settings.lfo1Unipolar = apvts.getRawParameterValue("lfo1Unipolar")->load();
    settings.lfo2Unipolar = apvts.getRawParameterValue("lfo2Unipolar")->load();
    settings.lfo3Unipolar = apvts.getRawParameterValue("lfo3Unipolar")->load();
    settings.filterKeytrack = apvts.getRawParameterValue("filterKeytrackEnable")->load();
    settings.filterKeytrackOffset = apvts.getRawParameterValue("filterKeytrackOffset")->load();


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
    tree.setProperty(IDs::ADSR3Attack, s.attackEnv3, nullptr);
    tree.setProperty(IDs::ADSR3Decay, s.decayEnv3, nullptr);
    tree.setProperty(IDs::ADSR3Sustain, s.sustainEnv3, nullptr);
    tree.setProperty(IDs::ADSR3Release, s.releaseEnv3, nullptr);
    tree.setProperty(IDs::SWgain, s.gain_osc1, nullptr);
    tree.setProperty(IDs::VAgain, s.gain_osc2, nullptr);
    tree.setProperty(IDs::LFODepth, s.lfodepth, nullptr);
    tree.setProperty(IDs::LFO2Depth, s.lfo2depth, nullptr);
    tree.setProperty(IDs::LFO3Depth, s.lfo3depth, nullptr);
    tree.setProperty(IDs::LFOFreq, s.lfofreq, nullptr);
    tree.setProperty(IDs::LFO2Freq, s.lfo2freq, nullptr);
    tree.setProperty(IDs::LFO3Freq, s.lfo3freq, nullptr);
    tree.setProperty(IDs::LFOType, s.lfoType, nullptr);
    tree.setProperty(IDs::LFO2Type, s.lfo2Type, nullptr);
    tree.setProperty(IDs::LFO3Type, s.lfo3Type, nullptr);
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
    tree.setProperty(IDs::GainOvr,s.gainOVR,nullptr);
    tree.setProperty(IDs::SWCoarse,s.detuneosc1,nullptr);
    tree.setProperty(IDs::VACoarse,s.detuneosc2,nullptr);
    tree.setProperty(IDs::ModDestination1,s.modDestination1,nullptr);
    tree.setProperty(IDs::ModSource1,s.modSource1,nullptr);
    tree.setProperty(IDs::ModIntensity1,s.modIntensity1,nullptr);
    tree.setProperty(IDs::ModDestination2,s.modDestination2,nullptr);
    tree.setProperty(IDs::ModSource2,s.modSource2,nullptr);
    tree.setProperty(IDs::ModIntensity2,s.modIntensity2,nullptr);
    tree.setProperty(IDs::ModDestination3,s.modDestination3,nullptr);
    tree.setProperty(IDs::ModSource3,s.modSource3,nullptr);
    tree.setProperty(IDs::ModIntensity3,s.modIntensity3,nullptr);
    tree.setProperty(IDs::ModDestination4,s.modDestination4,nullptr);
    tree.setProperty(IDs::ModSource4,s.modSource4,nullptr);
    tree.setProperty(IDs::ModIntensity4,s.modIntensity4,nullptr);
    tree.setProperty(IDs::LoopEnvelope,s.loopModEnvelope,nullptr);
    tree.setProperty(IDs::PulseWidthOSC1,s.pulseWidthOsc1,nullptr);
    tree.setProperty(IDs::PulseWidthOSC2,s.pulseWidthOsc2,nullptr);
    tree.setProperty(IDs::LFO1Unipolar,s.lfo1Unipolar,nullptr);
    tree.setProperty(IDs::LFO2Unipolar,s.lfo2Unipolar,nullptr);
    tree.setProperty(IDs::LFO3Unipolar,s.lfo3Unipolar,nullptr);
    tree.setProperty(IDs::FilterKeytrackEnable,s.filterKeytrack,nullptr);
    tree.setProperty(IDs::FilterKeytrackOffset,s.filterKeytrackOffset,nullptr);
}
