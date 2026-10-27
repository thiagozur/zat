#pragma once

#include <JuceHeader.h>
#include "../Styles/Palette.h"

class TypeSelector : public juce::Component
{
public:
    TypeSelector (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
        : param (*static_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramID))),
          attachment (param, [this] (float v) { index = juce::roundToInt (v); repaint(); })
    {
        attachment.sendInitialUpdate();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace ZatUI;

        const auto win = getWindow();
        const float h = win.getHeight();
        const float corner = 7.0f;

        juce::Path wp;
        wp.addRoundedRectangle (win, corner);
        juce::DropShadow (black.withAlpha (0.7f), 8, { 0, 3 }).drawForPath (g, wp);

        g.setColour (white.withAlpha (0.14f));
        g.fillRoundedRectangle (win.expanded (1.0f).translated (0.0f, 1.0f), corner + 1.0f);
        g.setGradientFill (juce::ColourGradient (onyx, 0.f, win.getY(),
            red.darker (2.4f), 0.f, win.getBottom(), false));
        g.fillPath (wp);

        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (wp);

            g.setGradientFill (juce::ColourGradient (red.withAlpha (0.28f), win.getCentreX(), win.getCentreY(),
                red.withAlpha (0.0f), win.getCentreX() + win.getWidth() * 0.55f, win.getCentreY(), true));
            g.fillRect (win);

            g.setGradientFill (juce::ColourGradient (white.withAlpha (0.10f), 0.f, win.getY(),
                white.withAlpha (0.0f), 0.f, win.getCentreY(), false));
            g.fillRect (win.withHeight (win.getHeight() * 0.5f));

            g.setGradientFill (juce::ColourGradient (black.withAlpha (0.55f), 0.f, win.getY(),
                black.withAlpha (0.0f), 0.f, win.getY() + h * 0.2f, false));
            g.fillRect (win);
        }

        g.setGradientFill (juce::ColourGradient (white.withAlpha (0.18f), 0.f, win.getY(),
            black.withAlpha (0.55f), 0.f, win.getBottom(), false));
        g.drawRoundedRectangle (win, corner, 1.2f);

        const juce::String name = juce::isPositiveAndBelow (index, param.choices.size())
            ? param.choices[index].toUpperCase() : juce::String();
        g.setFont (makeFont (h * 0.40f));
        const auto textArea = win.withTrimmedBottom (win.getHeight() * 0.26f).toNearestInt();
        for (auto off : { juce::Point<int> (-1, 0), juce::Point<int> (1, 0), juce::Point<int> (0, -1), juce::Point<int> (0, 1) })
        {
            g.setColour (red.withAlpha (0.4f));
            g.drawText (name, textArea.translated (off.x, off.y), juce::Justification::centred);
        }
        g.setColour (platinum);
        g.drawText (name, textArea, juce::Justification::centred);

        const int n = param.choices.size();
        if (n > 0)
        {
            const float spacing = h * 0.14f;
            const float lampR = h * 0.03f;
            const float startX = win.getCentreX() - spacing * (float) (n - 1) * 0.5f;
            const float ly = win.getBottom() - win.getHeight() * 0.15f;
            for (int i = 0; i < n; ++i)
            {
                const float lx = startX + spacing * (float) i;
                if (i == index)
                {
                    g.setColour (red.withAlpha (0.25f));
                    g.fillEllipse (lx - lampR * 2.4f, ly - lampR * 2.4f, lampR * 4.8f, lampR * 4.8f);
                    g.setColour (red.brighter (0.3f));
                }
                else
                    g.setColour (white.withAlpha (0.14f));
                g.fillEllipse (lx - lampR, ly - lampR, lampR * 2.f, lampR * 2.f);
            }
        }

        auto arrow = [&] (int dir, int zone)
        {
            const float s = h * 0.11f;
            const float cx = dir < 0 ? win.getX() + win.getWidth() * 0.09f : win.getRight() - win.getWidth() * 0.09f;
            const float cy = (float) textArea.getCentreY();
            const float tipX = cx + (float) dir * s * 0.5f, baseX = cx - (float) dir * s * 0.5f;

            juce::Path tri;
            tri.addTriangle (tipX, cy, baseX, cy - s * 0.8f, baseX, cy + s * 0.8f);
            g.setColour (platinum.withAlpha (pressedZone == zone ? 1.0f : (hover == zone ? 0.85f : 0.28f)));
            g.fillPath (tri);
        };
        arrow (-1, -1);
        arrow (+1, +1);
    }

    void mouseMove (const juce::MouseEvent& e) override { setHover (zoneAt (e.x)); }
    void mouseExit (const juce::MouseEvent&) override { setHover (2); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int z = zoneAt (e.x);
        if (z != 0) { pressedZone = z; step (z); repaint(); }
        else showMenu();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (pressedZone != 0) { pressedZone = 0; repaint(); }
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override
    {
        if (std::abs (w.deltaY) > 0.05f)
            step (w.deltaY > 0.f ? -1 : 1);
    }

private:
    juce::Rectangle<float> getWindow() const
    {
        return getLocalBounds().toFloat().reduced ((float) getHeight() * 0.1f);
    }

    int zoneAt (int x) const
    {
        const auto win = getWindow();
        if ((float) x < win.getX() + win.getWidth() * 0.2f) return -1;
        if ((float) x > win.getRight() - win.getWidth() * 0.2f) return +1;
        return 0;
    }

    void setHover (int z) { if (z != hover) { hover = z; repaint(); } }

    void choose (int newIndex) { attachment.setValueAsCompleteGesture ((float) newIndex); }

    void step (int dir)
    {
        const int n = param.choices.size();
        if (n > 0) choose ((index + dir + n) % n);
    }

    void showMenu()
    {
        juce::PopupMenu menu;
        menu.setLookAndFeel (&getLookAndFeel());
        for (int i = 0; i < param.choices.size(); ++i)
            menu.addItem (i + 1, param.choices[i], true, i == index);

        juce::Component::SafePointer<TypeSelector> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMinimumWidth (getWidth()),
            [safe] (int result)
            {
                if (safe != nullptr && result > 0)
                    safe->choose (result - 1);
            });
    }

    juce::AudioParameterChoice& param;
    juce::ParameterAttachment attachment;
    int index = 0;
    int hover = 2;
    int pressedZone = 0;
};