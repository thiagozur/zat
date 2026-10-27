#pragma once

#include <JuceHeader.h>

namespace ZatUI
{
    inline const juce::Colour onyx { 0xff0c090d };
    inline const juce::Colour platinum { 0xffefefef };
    inline const juce::Colour red = juce::Colour (0xffe01a4f).withMultipliedSaturation (1.08f);
    inline const juce::Colour blue = juce::Colour (0xff3454d1).withMultipliedSaturation (1.20f);
    inline const juce::Colour cyan = juce::Colour (0xff62929e).withMultipliedSaturation (1.50f);

    inline const juce::Colour white { 0xffffffff };
    inline const juce::Colour black { 0xff000000 };

    inline const juce::Colour redDeep = red.darker (1.8f);
    inline const juce::Colour hot = red.interpolatedWith (platinum, 0.4f);
    inline const juce::Colour textCol = platinum.darker (0.1f);

    inline juce::String fontName()
    {
        static const juce::String name = []() -> juce::String
        {
            const auto available = juce::Font::findAllTypefaceNames();
            for (auto* candidate : { "Bahnschrift", "DIN Alternate", "Avenir Next Condensed", "Arial Narrow", "Trebuchet MS" })
                if (available.contains (candidate))
                    return juce::String (candidate);
            return juce::Font::getDefaultSansSerifFontName();
        }();
        return name;
    }

    inline juce::Font makeFont (float height, bool bold = true)
    {
        return juce::Font (juce::FontOptions (fontName(), height, bold ? juce::Font::bold : juce::Font::plain));
    }

    inline constexpr float arcStart = juce::MathConstants<float>::pi * 1.25f;
    inline constexpr float arcEnd = juce::MathConstants<float>::pi * 2.75f;

    inline juce::Point<float> onCircle (juce::Point<float> c, float r, float angle)
    {
        return { c.x + r * std::sin (angle), c.y - r * std::cos (angle) };
    }

    inline juce::Colour heat (float h)
    {
        h = juce::jlimit (0.0f, 1.0f, h);
        return red.darker (0.25f).interpolatedWith (hot, h * h);
    }
}