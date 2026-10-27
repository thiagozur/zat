#pragma once

#include <JuceHeader.h>
#include "Palette.h"

class ZatLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ZatLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle, juce::Slider&) override;

    int getSliderThumbRadius (juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getPopupMenuFont() override;

private:
    void drawGlassKnob (juce::Graphics&, juce::Rectangle<float> bounds, float prop, float angle, float glow);
    void drawMiniKnob (juce::Graphics&, juce::Rectangle<float> bounds, float prop, float angle);
    void drawLamp (juce::Graphics&, juce::ToggleButton&, bool over, bool down);
    void drawAutoButton (juce::Graphics&, juce::ToggleButton&, bool over);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZatLookAndFeel)
};