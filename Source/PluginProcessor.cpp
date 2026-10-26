#include "PluginProcessor.h"
#include "PluginEditor.h"

ZatAudioProcessor::ZatAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    satTypeParam = apvts.getRawParameterValue (ParameterIDs::satType);
    satAmountParam = apvts.getRawParameterValue (ParameterIDs::satAmount);
    freqLowCutParam = apvts.getRawParameterValue (ParameterIDs::freqLowCut);
    freqHighCutParam = apvts.getRawParameterValue (ParameterIDs::freqHighCut);
    weightEnableParam = apvts.getRawParameterValue (ParameterIDs::weightEnable);
    weightAmountParam = apvts.getRawParameterValue (ParameterIDs::weightAmount);
    airEnableParam = apvts.getRawParameterValue (ParameterIDs::airEnable);
    airAmountParam = apvts.getRawParameterValue (ParameterIDs::airAmount);
    parallelModeParam = apvts.getRawParameterValue (ParameterIDs::parallelMode);
    outputGainParam = apvts.getRawParameterValue (ParameterIDs::outputGain);
}

ZatAudioProcessor::~ZatAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ZatAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    juce::StringArray satTypes { "Tube", "Tape", "Diode", "Soft Clip", "Transformer", "Solid State", "Rectifier", "Wavefolder" };
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParameterIDs::satType, 1 }, "Saturation Type", satTypes, 0
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::satAmount, 1 }, "Saturation Amount", 
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 1) + " %"; })
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::freqLowCut, 1 }, "Sat Low Cut",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 20.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 0) + " Hz"; })
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::freqHighCut, 1 }, "Sat High Cut",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 0) + " Hz"; })
        ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParameterIDs::weightEnable, 1 }, "Weight Enable", false
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::weightAmount, 1 }, "Weight Amount",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 1) + " %"; })
        ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParameterIDs::airEnable, 1 }, "Air Enable", false
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::airAmount, 1 }, "Air Amount",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 1) + " %"; })
        ));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParameterIDs::parallelMode, 1 }, "Parallel Send Only", false
    ));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::outputGain, 1 }, "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 1) + " dB"; })
    ));

    return layout;
}

void ZatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

}

void ZatAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ZatAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void ZatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /* midiMessages */)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

}

bool ZatAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* ZatAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

const juce::String ZatAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ZatAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ZatAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ZatAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ZatAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ZatAudioProcessor::getNumPrograms()
{
    return 1;
}

int ZatAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ZatAudioProcessor::setCurrentProgram (int /* index */)
{

}

const juce::String ZatAudioProcessor::getProgramName (int /* index */)
{
    return {};
}

void ZatAudioProcessor::changeProgramName (int /* index */, const juce::String& /* newName */)
{

}

void ZatAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ZatAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZatAudioProcessor();
}