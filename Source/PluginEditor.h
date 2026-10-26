#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ZatAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ZatAudioProcessorEditor (ZatAudioProcessor&);
    ~ZatAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ZatAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZatAudioProcessorEditor)
};