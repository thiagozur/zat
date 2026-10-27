#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Styles/Palette.h"
#include "Styles/ZatLookAndFeel.h"
#include "Components/TypeSelector.h"
#include "Components/GlassKnob.h"
#include "Components/EffectKnob.h"
#include "Components/FrequencyRangeSelector.h"
#include "Components/GainStrip.h"

class ZatAudioProcessorEditor : public juce::AudioProcessorEditor,
                                private juce::Timer
{
public:
    explicit ZatAudioProcessorEditor (ZatAudioProcessor&);
    ~ZatAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildBackground();

    ZatAudioProcessor& audioProcessor;

    ZatLookAndFeel laf;
    juce::TooltipWindow tooltipWindow { this, 600 };

    TypeSelector typeSelector;

    GlassKnob amountKnob;
    juce::AudioProcessorValueTreeState::SliderAttachment amountAttachment;

    EffectKnob weight, air;
    FrequencyRangeSelector range;
    GainStrip inStrip, outStrip;

    juce::Image background;
    juce::Rectangle<int> amountLabelArea;
    float glow = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZatAudioProcessorEditor)
};