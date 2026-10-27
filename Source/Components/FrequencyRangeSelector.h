#pragma once

#include <JuceHeader.h>
#include "../Styles/Palette.h"

class FrequencyRangeSelector : public juce::Component
{
public:
    FrequencyRangeSelector (juce::AudioProcessorValueTreeState& apvts,
        const juce::String& lowID, const juce::String& highID)
        : lowAttach (*apvts.getParameter (lowID), [this] (float v) { lowHz = v; repaint(); }),
          highAttach (*apvts.getParameter (highID), [this] (float v) { highHz = v; repaint(); })
    {
        lowAttach.sendInitialUpdate();
        highAttach.sendInitialUpdate();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace ZatUI;

        const auto a = barsArea();
        const float lp = hzToPos (lowHz), hp = hzToPos (highHz);

        const juce::Rectangle<float> litArea (posToX (lp), a.getY(), posToX (hp) - posToX (lp), a.getHeight());
        for (int i = 1; i <= 5; ++i)
        {
            g.setColour (red.withAlpha (0.022f));
            g.fillRoundedRectangle (litArea.expanded ((float) i * 2.0f), 3.0f + (float) i * 2.0f);
        }

        constexpr int N = 64;
        const float slot = a.getWidth() / (float) N;
        const float barW = slot * 0.58f;
        for (int i = 0; i < N; ++i)
        {
            const float centre = ((float) i + 0.5f) / (float) N;
            const bool lit = centre >= lp && centre <= hp;
            const juce::Rectangle<float> r (a.getX() + (float) i * slot + (slot - barW) * 0.5f, a.getY(), barW, a.getHeight());

            if (lit)
                g.setGradientFill (juce::ColourGradient (red.brighter (0.2f), 0.f, r.getY(),
                    red.darker (0.35f), 0.f, r.getBottom(), false));
            else
                g.setColour (redDeep.withAlpha (0.55f));
            g.fillRect (r);
        }

        const float h = (float) getHeight();
        g.setFont (makeFont (h * 0.13f));
        g.setColour (textCol.withAlpha (0.85f));
        const auto top = getLocalBounds().withHeight (juce::roundToInt (h * 0.17f));
        g.drawText ("LOW CUT  " + fmt (lowHz), top, juce::Justification::centredLeft);
        g.drawText ("HIGH CUT  " + fmt (highHz), top, juce::Justification::centredRight);

        g.setFont (makeFont (h * 0.14f));
        g.setColour (textCol.withAlpha (0.85f));
        static const char* names[] = { "20", "100", "250", "500", "1k", "2k", "5k", "10k", "20k" };
        const float ly = a.getBottom() + h * 0.06f;
        for (int i = 0; i < numLabels; ++i)
        {
            const float x = posToX (hzToPos (labelHz[i]));
            juce::Rectangle<float> r (60.f, h * 0.2f);
            auto just = juce::Justification::centred;
            if (i == 0) { r.setPosition (x, ly); just = juce::Justification::centredLeft; }
            else if (i == numLabels - 1) { r.setPosition (x - r.getWidth(), ly); just = juce::Justification::centredRight; }
            else { r.setPosition (x - r.getWidth() * 0.5f, ly); }
            g.drawText (names[i], r, just);
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const float x = e.position.x;
        const float lx = posToX (hzToPos (lowHz)), hx = posToX (hzToPos (highHz));
        if (std::abs (x - lx) < grab || std::abs (x - hx) < grab)
            setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
        else if (x > lx && x < hx)
            setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        else
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.getNumberOfClicks() > 1) return;

        const float x = e.position.x;
        const float lx = posToX (hzToPos (lowHz)), hx = posToX (hzToPos (highHz));
        const float dl = std::abs (x - lx), dh = std::abs (x - hx);

        if (dl < grab || dh < grab) drag = (dl <= dh) ? Drag::low : Drag::high;
        else if (x > lx && x < hx) { drag = Drag::both; grabOffset = xToPos (x) - hzToPos (lowHz); }
        else drag = (x < lx) ? Drag::low : Drag::high;

        if (drag != Drag::high) lowAttach.beginGesture();
        if (drag != Drag::low) highAttach.beginGesture();
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const float pos = xToPos (e.position.x);

        switch (drag)
        {
            case Drag::low:
                lowAttach.setValueAsPartOfGesture (juce::jmin (posToHz (pos), highHz - minSepHz));
                break;
            case Drag::high:
                highAttach.setValueAsPartOfGesture (juce::jmax (posToHz (pos), lowHz + minSepHz));
                break;
            case Drag::both:
            {
                const float width = hzToPos (highHz) - hzToPos (lowHz);
                const float newLowPos = juce::jlimit (0.0f, 1.0f - width, pos - grabOffset);
                const float nl = posToHz (newLowPos), nh = posToHz (newLowPos + width);
                if (nl > lowHz) { highAttach.setValueAsPartOfGesture (nh); lowAttach.setValueAsPartOfGesture (nl); }
                else { lowAttach.setValueAsPartOfGesture (nl); highAttach.setValueAsPartOfGesture (nh); }
                break;
            }
            case Drag::none: break;
        }
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (drag == Drag::none) return;
        if (drag != Drag::high) lowAttach.endGesture();
        if (drag != Drag::low) highAttach.endGesture();
        drag = Drag::none;
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        lowAttach.setValueAsCompleteGesture (20.0f);
        highAttach.setValueAsCompleteGesture (20000.0f);
    }

private:
    enum class Drag { none, low, high, both };

    juce::Rectangle<float> barsArea() const
    {
        const auto b = getLocalBounds().toFloat();
        return { b.getX() + 14.0f, b.getY() + b.getHeight() * 0.22f, b.getWidth() - 28.0f, b.getHeight() * 0.44f };
    }

    static float hzToPos (float hz) { return juce::jlimit (0.f, 1.f, std::log (hz / 20.f) / std::log (1000.f)); }
    static float posToHz (float p) { return 20.f * std::pow (1000.f, juce::jlimit (0.f, 1.f, p)); }

    float xToPos (float x) const { const auto a = barsArea(); return juce::jlimit (0.f, 1.f, (x - a.getX()) / a.getWidth()); }
    float posToX (float p) const { const auto a = barsArea(); return a.getX() + p * a.getWidth(); }

    static juce::String fmt (float hz)
    {
        return hz >= 1000.f ? juce::String (hz / 1000.f, 1) + " kHz" : juce::String (juce::roundToInt (hz)) + " Hz";
    }

    static constexpr int numLabels = 9;
    static constexpr float labelHz[numLabels] = { 20.f, 100.f, 250.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f };
    static constexpr float minSepHz = 100.0f;
    static constexpr float grab = 14.0f;

    float lowHz = 20.0f, highHz = 20000.0f;
    juce::ParameterAttachment lowAttach, highAttach;
    Drag drag = Drag::none;
    float grabOffset = 0.0f;
};