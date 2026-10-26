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
    inputGainParam = apvts.getRawParameterValue (ParameterIDs::inputGain);
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

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParameterIDs::inputGain, 1 }, "Input Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([](float val, int) { return juce::String (val, 1) + " dB"; })
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
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate * 4.0;
    spec.maximumBlockSize = static_cast<uint32_t> (samplesPerBlock * 4);
    spec.numChannels = static_cast<uint32_t> (getTotalNumOutputChannels());

    oversampling.initProcessing (samplesPerBlock);
    oversampling.reset();

    lowCutFilter.prepare (spec);
    highCutFilter.prepare (spec);
    weightFilter.prepare (spec);
    airFilter.prepare (spec);

    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 20.0f);
    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 20000.0f);
    *weightFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, 200.0f, 0.707f, 1.0f);
    *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 10000.0f, 0.707f, 1.0f);
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

float ZatAudioProcessor::applySaturation (float input, int type, float drive)
{
    float driveDb = drive * 24.0f;
    float preGain = juce::Decibels::decibelsToGain (driveDb);
    float x = input * preGain;

    switch (type)
    {
    case 0: // valvular
    {
        float biased = x + 0.15f * (x * x);
        return static_cast<float> (std::tanh (biased) / std::tanh (1.15));
    }
    case 1: // cinta
    {
        return std::atan (x * 1.2f) / std::atan (1.2f);
    }
    case 2: // diodo
    {
        if (x > 0.8f)
            return 0.8f + (x - 0.8f) / (1.0f + std::pow((x - 0.8f) / 0.2f, 2.0f));
        if (x < -0.6f)
            return -0.6f - (-x - 0.6f) / (1.0f + std::pow((-x - 0.6f) / 0.2f, 2.0f));
        return x;
    }
    case 3: // soft clipping con tgh
    {
        return std::tanh (x);
    }
    case 4: // transformador
    {
        float cubic = x - (x * x * x) / 3.0f;
        return std::tanh (cubic);
    }
    case 5: // estado sólido
    {
        float cubed = x * x * x;
        return x / (1.0f + std::abs (cubed) * 0.5f);
    }
    case 6: // rectificación
    {
        float rectified = std::abs (x) - 0.5f;
        return juce::jlimit (-1.0f, 1.0f, x + 0.3f * rectified);
    }
    case 7: // wavefolding
    {
        return std::sin (x * juce::MathConstants<float>::halfPi);
    }
    default:
        return std::tanh (x);
    }
}

void ZatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /* midiMessages */)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    int satType = static_cast<int> (*satTypeParam);
    
    float satAmount = *satAmountParam / 100.0f;
    
    float lowCutFreq = *freqLowCutParam;
    float highCutFreq = *freqHighCutParam;
    
    bool weightEnable = *weightEnableParam > 0.5f;
    float weightAmt = *weightAmountParam / 100.0f;
    
    bool airEnable = *airEnableParam > 0.5f;
    float airAmt = *airAmountParam / 100.0f;
    
    float inGainDb = *inputGainParam;
    float outGainDb = *outputGainParam;

    buffer.applyGain (juce::Decibels::decibelsToGain (inGainDb));

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    double oversampledRate = currentSampleRate * 4.0;

    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (oversampledRate, lowCutFreq);
    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (oversampledRate, highCutFreq);

    float weightGainDb = weightEnable ? (weightAmt * 12.0f) : 0.0f;
    *weightFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (oversampledRate, 180.0f, 0.707f, juce::Decibels::decibelsToGain (weightGainDb));
    
    float airGainDb = airEnable ? (airAmt * 15.0f) : 0.0f;
    *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (oversampledRate, 12000.0f, 0.707f, juce::Decibels::decibelsToGain (airGainDb));

    juce::dsp::AudioBlock<float> block (buffer);
    auto oversampledBlock = oversampling.processSamplesUp (block);

    juce::dsp::ProcessContextReplacing<float> context (oversampledBlock);

    lowCutFilter.process (context);
    highCutFilter.process (context);

    for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
    {
        auto* channelData = oversampledBlock.getChannelPointer (ch);
        for (size_t s = 0; s < oversampledBlock.getNumSamples(); ++s)
            channelData[s] = applySaturation (channelData[s], satType, satAmount);
    }

    weightFilter.process (context);
    airFilter.process (context);

    oversampling.processSamplesDown (block);

    const float equalPowerCoeff = 0.70710678f;

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* wet = buffer.getWritePointer (ch);
        const auto* dry = dryBuffer.getReadPointer (ch);

        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            wet[s] = (dry[s] * equalPowerCoeff) + (wet[s] * equalPowerCoeff);
        }
    }

    buffer.applyGain (juce::Decibels::decibelsToGain (outGainDb));
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