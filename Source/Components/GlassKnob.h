#pragma once

#include <JuceHeader.h>
#include "../Styles/Palette.h"

class GlassKnob : public juce::Slider
{
public:
    GlassKnob()
    {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        setRotaryParameters (ZatUI::arcStart, ZatUI::arcEnd, true);
        setMouseDragSensitivity (250);

        getProperties().set ("zatStyle", "glass");
        getProperties().set ("glow", 0.0f);
    }

    void setGlow (float v)
    {
        v = juce::jlimit (0.0f, 1.0f, v);
        if (std::abs (v - static_cast<float> (getProperties()["glow"])) > 0.004f)
        {
            getProperties().set ("glow", v);
            repaint();
        }
    }
};