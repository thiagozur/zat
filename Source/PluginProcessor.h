#pragma once

#include <JuceHeader.h>

namespace ParameterIDs
{
    static const juce::String satType = "satType";
    static const juce::String satAmount = "satAmount";
    static const juce::String freqLowCut = "freqLowCut";
    static const juce::String freqHighCut = "freqHighCut";

    static const juce::String weightEnable = "weightEnable";
    static const juce::String weightAmount = "weightAmount";
    
    static const juce::String airEnable = "airEnable";
    static const juce::String airAmount = "airAmount";

    static const juce::String inputGain = "inputGain";
    static const juce::String outputGain = "outputGain";
}

class ZatAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    ZatAudioProcessor();
    ~ZatAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float>* satTypeParam = nullptr;
    std::atomic<float>* satAmountParam = nullptr;
    std::atomic<float>* freqLowCutParam = nullptr;
    std::atomic<float>* freqHighCutParam = nullptr;
    std::atomic<float>* weightEnableParam = nullptr;
    std::atomic<float>* weightAmountParam = nullptr;
    std::atomic<float>* airEnableParam = nullptr;
    std::atomic<float>* airAmountParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;

    juce::dsp::Oversampling<float> oversampling { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true };

    using CutFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

    CutFilter weightFilter;
    CutFilter airFilter;

    juce::dsp::LinkwitzRileyFilter<float> lowCrossover;
    juce::dsp::LinkwitzRileyFilter<float> highCrossover;

    float applySaturation (float input, int type, float drive);

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZatAudioProcessor)
};