

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")

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
SuperWaveSynthAudioProcessor::SuperWaveSynthAudioProcessor()
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

    for (int i = 0; i < 16; ++i) {
        mySynth.addVoice(new SynthVoice(tree));
    }

    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());

    state.state.addListener(this);
}

SuperWaveSynthAudioProcessor::~SuperWaveSynthAudioProcessor()
{
    state.state.removeListener(this);
}

//==============================================================================
const juce::String SuperWaveSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SuperWaveSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SuperWaveSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SuperWaveSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SuperWaveSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SuperWaveSynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int SuperWaveSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SuperWaveSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SuperWaveSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SuperWaveSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SuperWaveSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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
    parametersChanged.store(true);
}

void SuperWaveSynthAudioProcessor::releaseResources()
{
   reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SuperWaveSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif
void SuperWaveSynthAudioProcessor::updateFilter() {
    *DCOffset.state = *juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(), 20.0f);
}
void SuperWaveSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {

        buffer.clear(i, 0, buffer.getNumSamples());
    }
    bool expected = true;
    if (isNonRealtime() || parametersChanged.compare_exchange_strong(expected, false)) {
        update();
        resetSynth = tree[IDs::Reset];
        if(resetSynth)
            reset();

        for (int i = 0; i < mySynth.getNumVoices(); i++) {
            if (myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))) {
                myVoice->update();
            }
        }
    }

    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);

    DCOffset.process(context);
    gainAmt.setTargetValue(static_cast<float>(tree[IDs::GainOvr]));

        for(auto sample=0;sample<buffer.getNumSamples();sample++)
        {
            pluginGain.setGainLinear(gainAmt.getNextValue());
            auto g = pluginGain.processSample(buffer.getSample(0,sample));
            auto g2 = pluginGain.processSample(buffer.getSample(1,sample));
            buffer.setSample(0,sample,g);
            buffer.setSample(1,sample,g2);
        }

}

void SuperWaveSynthAudioProcessor::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
    const juce::Identifier& property)
{
    parametersChanged.store(true);
}


//==============================================================================
bool SuperWaveSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SuperWaveSynthAudioProcessor::createEditor()
{
    return new SuperWaveSynthAudioProcessorEditor (*this);
}

//==============================================================================
void SuperWaveSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*state.copyState().createXml(), destData);

    DBG(state.copyState().toXmlString());
}

void SuperWaveSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(state.state.getType())) {
        state.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

void SuperWaveSynthAudioProcessor::resetAllParameters(juce::AudioProcessorValueTreeState& s)
{
    juce::ValueTree state = s.copyState();  // grab a copy of the current parameters Value Tree
    std::unique_ptr<juce::XmlElement> tempXml (state.createXml());  // convert parameters Value Tree to an XML object
    //iterate through XML elements with "PARAM" TAG
    for (auto* child : tempXml->getChildWithTagNameIterator("PARAM")){
        //get attribute of a parameter
        auto* param = s.getParameter(child->getStringAttribute("id"));
        if (param != nullptr)
        {
            //get default value without normalization to 0-1 range
            float defaultValue = param->convertFrom0to1(param->getDefaultValue());
            child->setAttribute("value", defaultValue);
        }
    }

    s.replaceState (juce::ValueTree::fromXml (*tempXml));

}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SuperWaveSynthAudioProcessor();
}
void SuperWaveSynthAudioProcessor::reset() {

    for (int i = 0; i < mySynth.getNumVoices(); i++) {

        if (myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))) {
            myVoice->reset();
        }
    }
    pluginGain.reset();
    DCOffset.reset();
    resetAllParameters(state);
    parametersChanged.store(true);
}

juce::AudioProcessorValueTreeState::ParameterLayout SuperWaveSynthAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto logRange = makeLogarithmicRange(20.0f, 20480.0f);
    juce::NormalisableRange<float> attackRange{0.0f,100.0f,1.f};
    juce::NormalisableRange<float> decayRange{0.0f,100.0f,1.f};
    juce::NormalisableRange<float> sustainRange{0.0f,100.0f,1.f};
    juce::NormalisableRange<float> releaseRange{0.0f,100.f,1.f};
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
    //Gain Params
    layout.add(std::make_unique<juce::AudioParameterFloat>(
    "gain_osc1",
    "GainOsc1",
    0.f,1.0f,0.6f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("panOsc1", "Pan OSC 1",-1.0f,1.0f,0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain_osc2", "GainOsc2", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("panOsc2", "Pan OSC 2", -1.0f, 1.0f, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterEnvelope", "Filter Envelope Amount", -100.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("filterEnvelope2", "Filter Envelope Amount", -100.0f, 100.0f, 0.0f));
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
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 1.f}, 0.0f));
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
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
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
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
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
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
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
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination4));
    auto attributesModSource4 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 4");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource4", "Mod Source 4",
    juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    2, attributesModDestination4));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity4","Mod Intensity 4",
        juce::NormalisableRange<float>(0.f,100.f,1.f),0.f));

    auto attributesModDestination5 = juce::AudioParameterChoiceAttributes().withLabel("MOD Destination 5");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modDestination5", "Mod Destination 5",
    juce::StringArray{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination5));
    auto attributesModSource5 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 5");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource5", "Mod Source 5",
    juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    2, attributesModDestination5));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity5","Mod Intensity 5",
        juce::NormalisableRange<float>(0.f,100.f,1.f),0.f));

    auto attributesModDestination6 = juce::AudioParameterChoiceAttributes().withLabel("MOD Destination 6");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modDestination6", "Mod Destination 6",
    juce::StringArray{ "No connection","Filter CutOff","Filter Resonance","Detune Volume","Detune Amount","Pitch OSC1","Pitch OSC2", "Gain OSC1",
        "Gain OSC2","Pan OSC1","Pan OSC2","LFO1 Amount","LFO2 Amount","LFO3 Amount","LFO1 Frequency","LFO2 Frequency","LFO3 Frequency","OSC1 PWM",
    "OSC2 PWM", "OSC1 TYPE","OSC2 TYPE"},
    0, attributesModDestination6));
    auto attributesModSource6 = juce::AudioParameterChoiceAttributes().withLabel("MOD Source 6");
    layout.add(std::make_unique<juce::AudioParameterChoice>("modSource6", "Mod Source 6",
    juce::StringArray{"LFO 1","LFO2","LFO3","AMP","Env2","Env3"},
    4, attributesModDestination5));
    layout.add(std::make_unique<juce::AudioParameterFloat>("modIntensity6","Mod Intensity 6",
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
    layout.add(std::make_unique<juce::AudioParameterBool>("lfoReset2", "LFO2 Reset", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfoReset3", "LFO3 Reset", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("commonEnvelope", "Env2->OSC2", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("loopEnvelope", "Loop Mod Envelope1", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("loopEnvelope2", "Loop Mod Envelope2", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfo1Unipolar", "LFO1 Unipolar", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfo2Unipolar", "LFO2 Unipolar", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("lfo3Unipolar", "LFO3 Unipolar", 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("reset", "Init", 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("lfoNumber","Selected LFO",1,3,1));
    layout.add(std::make_unique<juce::AudioParameterBool>("aliasing","Aliasing ON",0));


    return layout;
}
SuperWaveSynthAudioProcessor::chainSettings SuperWaveSynthAudioProcessor::getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    SuperWaveSynthAudioProcessor::chainSettings settings;

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
    settings.envelopeAmount2 = apvts.getRawParameterValue("filterEnvelope2")->load();
    settings.gain_osc1 = apvts.getRawParameterValue("gain_osc1")->load();
    settings.gain_osc2 = apvts.getRawParameterValue("gain_osc2")->load();
    settings.lfodepth = apvts.getRawParameterValue("lfodepth")->load();
    settings.lfo2depth = apvts.getRawParameterValue("lfo2depth")->load();
    settings.lfo3depth = apvts.getRawParameterValue("lfo3depth")->load();
    settings.lfofreq = apvts.getRawParameterValue("lfofreq")->load();
    settings.lfo2freq = apvts.getRawParameterValue("lfo2freq")->load();
    settings.lfo3freq = apvts.getRawParameterValue("lfo3freq")->load();
    settings.lfoReset = apvts.getRawParameterValue("lfoReset")->load();
    settings.lfoReset2 = apvts.getRawParameterValue("lfoReset2")->load();
    settings.lfoReset3 = apvts.getRawParameterValue("lfoReset3")->load();
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
    settings.modDestination5 = apvts.getRawParameterValue("modDestination5")->load();
    settings.modDestination6 = apvts.getRawParameterValue("modDestination6")->load();
    settings.modSource4 = apvts.getRawParameterValue("modSource4")->load();
    settings.modSource5 = apvts.getRawParameterValue("modSource5")->load();
    settings.modSource6 = apvts.getRawParameterValue("modSource6")->load();
    settings.modIntensity4 = apvts.getRawParameterValue("modIntensity4")->load();
    settings.modIntensity5 = apvts.getRawParameterValue("modIntensity5")->load();
    settings.modIntensity6 = apvts.getRawParameterValue("modIntensity6")->load();
    settings.loopModEnvelope = apvts.getRawParameterValue("loopEnvelope")->load();
    settings.loopModEnvelope2 = apvts.getRawParameterValue("loopEnvelope2")->load();
    settings.pulseWidthOsc1 = apvts.getRawParameterValue("pulseWidthOsc1")->load();
    settings.pulseWidthOsc2 = apvts.getRawParameterValue("pulseWidthOsc2")->load();
    settings.lfo1Unipolar = apvts.getRawParameterValue("lfo1Unipolar")->load();
    settings.lfo2Unipolar = apvts.getRawParameterValue("lfo2Unipolar")->load();
    settings.lfo3Unipolar = apvts.getRawParameterValue("lfo3Unipolar")->load();
    settings.filterKeytrack = apvts.getRawParameterValue("filterKeytrackEnable")->load();
    settings.filterKeytrackOffset = apvts.getRawParameterValue("filterKeytrackOffset")->load();
    settings.reset = apvts.getRawParameterValue("reset")->load();
    settings.lfoNumber = apvts.getRawParameterValue("lfoNumber")->load();
    settings.aliasingON = apvts.getRawParameterValue("aliasing")->load();


    return settings;
}

juce::ValueTree SuperWaveSynthAudioProcessor::createValueTree()
{
    juce::ValueTree v(IDs::Parameters);
    return v;

}

void SuperWaveSynthAudioProcessor::syncStates(juce::ValueTree& tree,chainSettings& s)
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
    tree.setProperty(IDs::LFOReset2,s.lfoReset2,nullptr);
    tree.setProperty(IDs::LFOReset3,s.lfoReset3,nullptr);
    tree.setProperty(IDs::CommonEnvelope,s.commonEnvelope,nullptr);
    tree.setProperty(IDs::FilterEnvelopeAmount,s.envelopeAmount,nullptr);
    tree.setProperty(IDs::FilterEnvelopeAmount2,s.envelopeAmount2,nullptr);
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
    tree.setProperty(IDs::ModDestination5,s.modDestination5,nullptr);
    tree.setProperty(IDs::ModDestination6,s.modDestination6,nullptr);
    tree.setProperty(IDs::ModSource4,s.modSource4,nullptr);
    tree.setProperty(IDs::ModSource5,s.modSource5,nullptr);
    tree.setProperty(IDs::ModSource6,s.modSource6,nullptr);
    tree.setProperty(IDs::ModIntensity4,s.modIntensity4,nullptr);
    tree.setProperty(IDs::ModIntensity5,s.modIntensity5,nullptr);
    tree.setProperty(IDs::ModIntensity6,s.modIntensity6,nullptr);
    tree.setProperty(IDs::LoopEnvelope,s.loopModEnvelope,nullptr);
    tree.setProperty(IDs::LoopEnvelope2,s.loopModEnvelope2,nullptr);
    tree.setProperty(IDs::PulseWidthOSC1,s.pulseWidthOsc1,nullptr);
    tree.setProperty(IDs::PulseWidthOSC2,s.pulseWidthOsc2,nullptr);
    tree.setProperty(IDs::LFO1Unipolar,s.lfo1Unipolar,nullptr);
    tree.setProperty(IDs::LFO2Unipolar,s.lfo2Unipolar,nullptr);
    tree.setProperty(IDs::LFO3Unipolar,s.lfo3Unipolar,nullptr);
    tree.setProperty(IDs::FilterKeytrackEnable,s.filterKeytrack,nullptr);
    tree.setProperty(IDs::FilterKeytrackOffset,s.filterKeytrackOffset,nullptr);
    tree.setProperty(IDs::Reset,s.reset,nullptr);
    tree.setProperty(IDs::AliasingON,s.aliasingON,nullptr);
}

void SuperWaveSynthAudioProcessor::update()
{
    auto s = getChainSettings(state);
    float inverseSampleRate = 1.0f / getSampleRate();
    s.attack = static_cast<float> (std::exp(-inverseSampleRate *
            std::exp(5.5f - 0.075f * s.attack)));
    s.decay = std::exp(-inverseSampleRate *
        std::exp(5.5f - 0.075f * s.decay));
    s.sustain = s.sustain/100.f;
    float envRelease = s.release;
    if (envRelease < 1.0f) {
        s.release = 0.75f;  // extra fast release
    } else {
        s.release = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envRelease));
    }

    s.attackEnv2 = static_cast<float> (std::exp(-inverseSampleRate *
            std::exp(5.5f - 0.075f * s.attackEnv2)));
    s.decayEnv2 = std::exp(-inverseSampleRate *
        std::exp(5.5f - 0.075f * s.decayEnv2));
    s.sustainEnv2 = s.sustainEnv2/100.f;
    envRelease = s.releaseEnv2;
    if (envRelease < 1.0f) {
        s.releaseEnv2 = 0.75f;  // extra fast release
    } else {
        s.releaseEnv2 = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envRelease));
    }

    s.attackEnv3 = static_cast<float> (std::exp(-inverseSampleRate *
            std::exp(5.5f - 0.075f * s.attackEnv3)));
    s.decayEnv3 = std::exp(-inverseSampleRate *
        std::exp(5.5f - 0.075f * s.decayEnv3));
    s.sustainEnv3 = s.sustainEnv3/100.f;
    envRelease = s.releaseEnv3;
    if (envRelease < 1.0f) {
        s.releaseEnv3 = 0.75f;  // extra fast release
    } else {
        s.releaseEnv3 = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envRelease));
    }
    syncStates(tree,s);
}
