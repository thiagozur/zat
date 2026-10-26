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

    apvts.addParameterListener (ParameterIDs::freqLowCut, this);
    apvts.addParameterListener (ParameterIDs::freqHighCut, this);
}

ZatAudioProcessor::~ZatAudioProcessor()
{
    apvts.removeParameterListener (ParameterIDs::freqLowCut, this);
    apvts.removeParameterListener (ParameterIDs::freqHighCut, this);
}

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

void ZatAudioProcessor::parameterChanged (const juce::String& parameterID, float /* newValue */)
{
    if (parameterID == ParameterIDs::freqLowCut || parameterID == ParameterIDs::freqHighCut)
    {
        auto lowParam = apvts.getParameter (ParameterIDs::freqLowCut);
        auto highParam = apvts.getParameter (ParameterIDs::freqHighCut);

        float lowHz = *apvts.getRawParameterValue (ParameterIDs::freqLowCut);
        float highHz = *apvts.getRawParameterValue (ParameterIDs::freqHighCut);

        const float minSep = 100.0f;

        if (lowHz > highHz - minSep)
        {
            if (parameterID == ParameterIDs::freqLowCut)
            {
                float newHigh = juce::jmin (20000.0f, lowHz + minSep);
                highParam->setValueNotifyingHost (highParam->convertTo0to1 (newHigh));
            }
            else
            {
                float newLow = juce::jmax (20.0f, highHz - minSep);
                lowParam->setValueNotifyingHost (lowParam->convertTo0to1 (newLow));
            }
        }
    }
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

    lowCrossover.prepare (spec);
    highCrossover.prepare (spec);
    
    lowCrossover.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    highCrossover.setType (juce::dsp::LinkwitzRileyFilterType::highpass);

    lowCrossover.setCutoffFrequency (20.0f);
    highCrossover.setCutoffFrequency (20000.0f);
    lowCrossover.reset();
    highCrossover.reset();

    weightFilter.prepare (spec);
    airFilter.prepare (spec);

    *weightFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate * 4.0, 200.0f, 0.707f, 1.0f);
    *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate * 4.0, 10000.0f, 0.707f, 1.0f);
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
        float bias = 0.05f + 0.2f * drive;
        float biased = x + bias * (x * x);
        float denominator = std::tanh (1.0f + bias);
        return static_cast<float> (std::tanh (biased) / (denominator > 0.001f ? denominator : 1.0f));
    }
    case 1: // cinta
    {
        float curve = 1.0f + 0.5f * drive;
        return std::atan (x * curve) / std::atan (curve);
    }
    case 2: // diodo
    {
        float threshold = 0.9f - 0.2f * drive;
        if (x > threshold)
            return threshold + (x - threshold) / (1.0f + std::pow((x - threshold) / 0.2f, 2.0f));
        if (x < -threshold)
            return -threshold - (-x - threshold) / (1.0f + std::pow((-x - threshold) / 0.2f, 2.0f));
        return x;
    }
    case 3: // soft clipping con tgh
    {
        float sharpness = 1.0f + 0.5f * drive;
        return std::tanh (x * sharpness) / sharpness;
    }
    case 4: // transformador
    {
        float cubicCoeff = 0.2f + 0.15f * drive;
        float cubic = x - cubicCoeff * (x * x * x);
        return std::tanh (cubic);
    }
    case 5: // estado sólido
    {
        float scale = 0.3f + 0.4f * drive;
        float cubed = x * x * x;
        return x / (1.0f + std::abs (cubed) * scale);
    }
    case 6: // rectificación
    {
        float rectMix = 0.15f + 0.3f * drive;
        float rectified = std::abs (x) - 0.5f;
        return juce::jlimit (-1.0f, 1.0f, x + rectMix * rectified);
    }
    case 7: // wavefolding
    {
        float foldScale = 1.0f + 1.0f * drive;
        return std::sin (x * foldScale * juce::MathConstants<float>::halfPi) / foldScale;
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

    double oversampledRate = currentSampleRate * 4.0;

    lowCrossover.setCutoffFrequency (lowCutFreq);
    highCrossover.setCutoffFrequency (highCutFreq);

    float weightGainDb = weightEnable ? (weightAmt * 12.0f) : 0.0f;
    *weightFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (oversampledRate, 180.0f, 0.707f, juce::Decibels::decibelsToGain (weightGainDb));
    
    float airGainDb = airEnable ? (airAmt * 15.0f) : 0.0f;
    *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (oversampledRate, 12000.0f, 0.707f, juce::Decibels::decibelsToGain (airGainDb));

    juce::dsp::AudioBlock<float> block (buffer);
    auto oversampledBlock = oversampling.processSamplesUp (block);

    int numCh = static_cast<int> (oversampledBlock.getNumChannels());
    int numSamps = static_cast<int> (oversampledBlock.getNumSamples());

    bool bypassCrossover = (std::abs (lowCutFreq - 20.0f) < 0.1f && std::abs (highCutFreq - 20000.0f) < 0.1f);

    if (bypassCrossover)
    {
        juce::dsp::ProcessContextReplacing<float> context (oversampledBlock);

        for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
        {
            auto* channelData = oversampledBlock.getChannelPointer (ch);
            for (size_t s = 0; s < oversampledBlock.getNumSamples(); ++s)
                channelData[s] = applySaturation (channelData[s], satType, satAmount);
        }

        weightFilter.process (context);

        if (weightEnable && weightAmt > 0.001f)
        {
            for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
            {
                auto* channelData = oversampledBlock.getChannelPointer (ch);
                for (size_t s = 0; s < oversampledBlock.getNumSamples(); ++s)
                {
                    float sample = channelData[s];
                    float harmonic = sample + 0.15f * weightAmt * (sample * sample - sample * sample * sample);
                    channelData[s] = sample + weightAmt * 0.3f * (harmonic - sample);
                }
            }
        }

        airFilter.process (context);

        if (airEnable && airAmt > 0.001f)
        {
            for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
            {
                auto* channelData = oversampledBlock.getChannelPointer (ch);
                for (size_t s = 0; s < oversampledBlock.getNumSamples(); ++s)
                {
                    float sample = channelData[s];
                    float exciter = std::tanh (sample * 1.5f) - sample;
                    channelData[s] = sample + airAmt * 0.4f * exciter;
                }
            }
        }
    }
    else
    {
        juce::AudioBuffer<float> lowBuffer (numCh, numSamps);
        juce::AudioBuffer<float> midBuffer (numCh, numSamps);
        juce::AudioBuffer<float> highBuffer (numCh, numSamps);

        for (int ch = 0; ch < numCh; ++ch)
        {
            juce::FloatVectorOperations::copy (lowBuffer.getWritePointer (ch), oversampledBlock.getChannelPointer (ch), numSamps);
            juce::FloatVectorOperations::copy (midBuffer.getWritePointer (ch), oversampledBlock.getChannelPointer (ch), numSamps);
            juce::FloatVectorOperations::copy (highBuffer.getWritePointer (ch), oversampledBlock.getChannelPointer (ch), numSamps);
        }

        juce::dsp::AudioBlock<float> lowBlock (lowBuffer);
        juce::dsp::AudioBlock<float> midBlock (midBuffer);
        juce::dsp::AudioBlock<float> highBlock (highBuffer);

        lowCrossover.process (juce::dsp::ProcessContextReplacing<float> (lowBlock));
        highCrossover.process (juce::dsp::ProcessContextReplacing<float> (highBlock));

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* orig = oversampledBlock.getChannelPointer (ch);
            auto* low = lowBuffer.getReadPointer (ch);
            auto* high = highBuffer.getReadPointer (ch);
            auto* mid = midBuffer.getWritePointer (ch);

            for (int s = 0; s < numSamps; ++s)
            {
                mid[s] = orig[s] - low[s] - high[s];
            }
        }

        juce::dsp::ProcessContextReplacing<float> midContext (midBlock);

        for (size_t ch = 0; ch < midBlock.getNumChannels(); ++ch)
        {
            auto* channelData = midBlock.getChannelPointer (ch);
            for (size_t s = 0; s < midBlock.getNumSamples(); ++s)
                channelData[s] = applySaturation (channelData[s], satType, satAmount);
        }

        weightFilter.process (midContext);

        if (weightEnable && weightAmt > 0.001f)
        {
            for (size_t ch = 0; ch < midBlock.getNumChannels(); ++ch)
            {
                auto* channelData = midBlock.getChannelPointer (ch);
                for (size_t s = 0; s < midBlock.getNumSamples(); ++s)
                {
                    float sample = channelData[s];
                    float harmonic = sample + 0.15f * weightAmt * (sample * sample - sample * sample * sample);
                    channelData[s] = sample + weightAmt * 0.3f * (harmonic - sample);
                }
            }
        }

        airFilter.process (midContext);

        if (airEnable && airAmt > 0.001f)
        {
            for (size_t ch = 0; ch < midBlock.getNumChannels(); ++ch)
            {
                auto* channelData = midBlock.getChannelPointer (ch);
                for (size_t s = 0; s < midBlock.getNumSamples(); ++s)
                {
                    float sample = channelData[s];
                    float exciter = std::tanh (sample * 1.5f) - sample;
                    channelData[s] = sample + airAmt * 0.4f * exciter;
                }
            }
        }

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* dest = oversampledBlock.getChannelPointer (ch);
            auto* low = lowBuffer.getReadPointer (ch);
            auto* mid = midBuffer.getReadPointer (ch);
            auto* high = highBuffer.getReadPointer (ch);

            for (int s = 0; s < numSamps; ++s)
            {
                dest[s] = low[s] + mid[s] + high[s];
            }
        }
    }

    oversampling.processSamplesDown (block);

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