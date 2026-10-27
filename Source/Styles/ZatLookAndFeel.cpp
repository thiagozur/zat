#include "ZatLookAndFeel.h"

using namespace ZatUI;

ZatLookAndFeel::ZatLookAndFeel()
{
    setDefaultSansSerifTypefaceName (fontName());

    setColour (juce::PopupMenu::backgroundColourId, onyx.interpolatedWith (platinum, 0.08f));
    setColour (juce::PopupMenu::textColourId, platinum);
    setColour (juce::PopupMenu::headerTextColourId, cyan);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, red.darker (0.2f));
    setColour (juce::PopupMenu::highlightedTextColourId, platinum);

    setColour (juce::TooltipWindow::backgroundColourId, onyx.interpolatedWith (platinum, 0.1f));
    setColour (juce::TooltipWindow::textColourId, platinum);
    setColour (juce::TooltipWindow::outlineColourId, cyan.withAlpha (0.5f));
}

juce::Font ZatLookAndFeel::getPopupMenuFont()
{
    return makeFont (15.0f, false);
}

int ZatLookAndFeel::getSliderThumbRadius (juce::Slider& s)
{
    if (s.isVertical())
        return juce::roundToInt ((float) s.getHeight() * 3.0f / 22.0f);

    return LookAndFeel_V4::getSliderThumbRadius (s);
}

void ZatLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
    float pos, float startAngle, float endAngle, juce::Slider& s)
{
    const auto style = s.getProperties()["zatStyle"].toString();
    const juce::Rectangle<float> b ((float) x, (float) y, (float) width, (float) height);
    const float angle = startAngle + pos * (endAngle - startAngle);

    if (style == "glass")
        drawGlassKnob (g, b, pos, angle, static_cast<float> (s.getProperties()["glow"]));
    else if (style == "mini")
        drawMiniKnob (g, b, pos, angle);
    else
        LookAndFeel_V4::drawRotarySlider (g, x, y, width, height, pos, startAngle, endAngle, s);
}

void ZatLookAndFeel::drawGlassKnob (juce::Graphics& g, juce::Rectangle<float> b, float prop, float angle, float glow)
{
    const float R = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const auto glowCol = heat (glow);

    for (int i = 0; i <= 60; ++i)
    {
        const float t = (float) i / 60.0f;
        const float a = arcStart + t * (arcEnd - arcStart);
        const bool major = (i % 5 == 0);
        const auto p1 = onCircle (c, R * (major ? 0.915f : 0.935f), a);
        const auto p2 = onCircle (c, R * 0.965f, a);
        g.setColour (t <= prop ? glowCol.withAlpha (0.4f + 0.5f * glow)
            : platinum.withAlpha (major ? 0.2f : 0.1f));
        g.drawLine (juce::Line<float> (p1, p2), major ? 1.6f : 1.0f);
    }

    const float ringR = R * 0.84f;
    const float ringW = R * 0.05f;
    juce::Path track;
    track.addCentredArc (c.x, c.y, ringR, ringR, 0.f, arcStart, arcEnd, true);
    g.setColour (white.withAlpha (0.06f));
    g.strokePath (track, juce::PathStrokeType (ringW, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

    if (prop > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, ringR, ringR, 0.f, arcStart, angle, true);
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (glowCol.withAlpha (0.05f + glow * 0.10f));
            g.strokePath (val, juce::PathStrokeType (ringW + (float) i * R * 0.035f,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour (glowCol.brighter (0.1f));
        g.strokePath (val, juce::PathStrokeType (ringW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float bodyR = R * 0.74f;
    juce::Path body;
    body.addEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2.f, bodyR * 2.f);

    juce::DropShadow (black.withAlpha (0.8f), juce::roundToInt (R * 0.12f),
        { 0, juce::roundToInt (R * 0.05f) }).drawForPath (g, body);

    g.setGradientFill (juce::ColourGradient (onyx.interpolatedWith (platinum, 0.1f), c.x, c.y - bodyR,
        onyx, c.x, c.y + bodyR, false));
    g.fillPath (body);

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (body);

        const float gr = bodyR * (0.55f + 0.5f * glow);
        const auto gc = c.translated (0.f, bodyR * 0.12f);
        g.setGradientFill (juce::ColourGradient (glowCol.withAlpha (0.16f + 0.72f * glow), gc.x, gc.y,
            glowCol.withAlpha (0.0f), gc.x + gr, gc.y, true));
        g.fillPath (body);

        g.setGradientFill (juce::ColourGradient (hot.withAlpha (glow * glow * 0.8f), c.x, c.y,
            hot.withAlpha (0.0f), c.x + bodyR * 0.35f, c.y, true));
        g.fillPath (body);

        auto hl = juce::Rectangle<float> (bodyR * 1.5f, bodyR * 0.8f).withCentre ({ c.x, c.y - bodyR * 0.52f });
        g.setGradientFill (juce::ColourGradient (white.withAlpha (0.26f), 0.f, hl.getY(),
            white.withAlpha (0.0f), 0.f, hl.getBottom(), false));
        g.fillEllipse (hl);

        juce::Path lowArc;
        lowArc.addCentredArc (c.x, c.y, bodyR * 0.93f, bodyR * 0.93f, 0.f,
            juce::MathConstants<float>::pi * 0.62f, juce::MathConstants<float>::pi * 1.38f, true);
        g.setColour (glowCol.withAlpha (0.18f + 0.4f * glow));
        g.strokePath (lowArc, juce::PathStrokeType (bodyR * 0.05f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setGradientFill (juce::ColourGradient (white.withAlpha (0.5f), 0.f, c.y - bodyR,
        black.withAlpha (0.6f), 0.f, c.y + bodyR, false));
    g.drawEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2.f, bodyR * 2.f, 1.6f);

    juce::Path pill;
    const float pw = bodyR * 0.07f, ph = bodyR * 0.17f;
    pill.addRoundedRectangle (c.x - pw * 0.5f, c.y - bodyR * 0.82f, pw, ph, pw * 0.5f);
    pill.applyTransform (juce::AffineTransform::rotation (angle, c.x, c.y));
    juce::DropShadow (glowCol.withAlpha (0.9f), juce::roundToInt (R * 0.05f), { 0, 0 }).drawForPath (g, pill);
    g.setColour (glowCol.brighter (0.4f));
    g.fillPath (pill);
}

void ZatLookAndFeel::drawMiniKnob (juce::Graphics& g, juce::Rectangle<float> b, float , float angle)
{
    const auto c = b.getCentre();
    const float full = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const float R = full * 0.86f;

    juce::Path disc;
    disc.addEllipse (c.x - R, c.y - R, R * 2.f, R * 2.f);
    juce::DropShadow (black.withAlpha (0.8f), juce::roundToInt (R * 0.45f),
        { 0, juce::roundToInt (R * 0.16f) }).drawForPath (g, disc);

    g.setGradientFill (juce::ColourGradient (platinum.darker (0.5f), c.x, c.y - R,
        onyx.interpolatedWith (platinum, 0.25f), c.x, c.y + R, false));
    g.fillPath (disc);
    for (int i = 0; i < 72; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 72.0f;
        g.setColour ((i & 1) ? black.withAlpha (0.38f) : white.withAlpha (0.14f));
        g.drawLine (juce::Line<float> (onCircle (c, R * 0.9f, a), onCircle (c, R, a)), 1.0f);
    }

    const float capR = R * 0.8f;
    constexpr int N = 180;
    const auto dark = platinum.interpolatedWith (cyan, 0.15f).darker (1.3f);
    const auto light = platinum.brighter (0.1f);
    for (int i = 0; i < N; ++i)
    {
        const float a0 = (float) i * juce::MathConstants<float>::twoPi / (float) N;
        const float a1 = (float) (i + 1) * juce::MathConstants<float>::twoPi / (float) N + 0.01f;
        const float l = 0.25f + 0.75f * std::pow (std::abs (std::cos (a0 - 0.55f)), 4.0f);

        juce::Path wedge;
        wedge.addPieSegment (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f, a0, a1, 0.0f);
        g.setColour (dark.interpolatedWith (light, l));
        g.fillPath (wedge);
    }

    for (int k = 1; k <= 7; ++k)
    {
        const float rr = capR * (float) k / 8.0f;
        g.setColour ((k & 1) ? white.withAlpha (0.05f) : black.withAlpha (0.05f));
        g.drawEllipse (c.x - rr, c.y - rr, rr * 2.f, rr * 2.f, 1.0f);
    }

    g.setGradientFill (juce::ColourGradient (white.withAlpha (0.22f), c.x - capR * 0.6f, c.y - capR * 0.8f,
        white.withAlpha (0.0f), c.x + capR * 0.2f, c.y + capR * 0.2f, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f);

    g.setColour (black.withAlpha (0.35f));
    g.drawEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f, 1.0f);

    juce::Path p;
    p.startNewSubPath (onCircle ({ 0.f, 0.f }, capR * 0.32f, 0.f));
    p.lineTo (onCircle ({ 0.f, 0.f }, capR * 0.9f, 0.f));
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (c.x, c.y));
    g.setColour (onyx);
    g.strokePath (p, juce::PathStrokeType (R * 0.11f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void ZatLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    const juce::Rectangle<float> b ((float) x, (float) y, (float) width, (float) height);
    const float cx = b.getCentreX();
    const float tr = (float) getSliderThumbRadius (slider);
    const float capH = tr * 2.0f;
    const float capW = b.getWidth() * 0.62f;

    g.beginTransparencyLayer (slider.isEnabled() ? 1.0f : 0.6f);

    for (int db = -24; db <= 24; db += 6)
    {
        const float ty = slider.getPositionOfValue ((double) db);
        const bool zero = (db == 0);
        const float len = zero ? 8.0f : 5.0f;
        g.setColour (platinum.withAlpha (zero ? 0.4f : 0.2f));
        g.fillRect (cx - capW * 0.5f - 4.0f - len, ty - 0.5f, len, 1.0f);
        g.fillRect (cx + capW * 0.5f + 4.0f, ty - 0.5f, len, 1.0f);
    }

    juce::Rectangle<float> slot (6.0f, b.getHeight() + tr * 1.4f);
    slot.setCentre (b.getCentre());
    g.setColour (onyx.darker (0.5f));
    g.fillRoundedRectangle (slot, 2.5f);
    g.setColour (white.withAlpha (0.09f));
    g.drawRoundedRectangle (slot, 2.5f, 1.0f);

    const float zeroY = slider.getPositionOfValue (0.0);
    const float top = juce::jmin (zeroY, sliderPos), bottom = juce::jmax (zeroY, sliderPos);
    if (bottom - top > 0.5f)
    {
        g.setColour (red.withAlpha (0.28f));
        g.fillRoundedRectangle (slot.getX() - 2.f, top, slot.getWidth() + 4.f, bottom - top, 3.f);
        g.setColour (red);
        g.fillRoundedRectangle (slot.getX(), top, slot.getWidth(), bottom - top, 2.f);
    }

    juce::Rectangle<float> cap (capW, capH);
    cap.setCentre (cx, sliderPos);
    juce::Path cp;
    cp.addRoundedRectangle (cap, capW * 0.12f);

    juce::DropShadow (black.withAlpha (0.8f), 9, { 0, 3 }).drawForPath (g, cp);
    g.setGradientFill (juce::ColourGradient (platinum.brighter (0.05f), 0.f, cap.getY(),
        platinum.interpolatedWith (cyan, 0.15f).darker (1.1f), 0.f, cap.getBottom(), false));
    g.fillPath (cp);

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (cp);

        for (int i = -5; i <= 5; ++i)
        {
            if (i == 0) continue;
            const float yy = cap.getCentreY() + (float) i * capH * 0.075f;
            g.setColour (black.withAlpha (0.22f));
            g.fillRect (cap.getX(), yy, cap.getWidth(), 1.2f);
            g.setColour (white.withAlpha (0.35f));
            g.fillRect (cap.getX(), yy + 1.2f, cap.getWidth(), 1.0f);
        }

        juce::ColourGradient side (black.withAlpha (0.3f), cap.getX(), 0.f, black.withAlpha (0.3f), cap.getRight(), 0.f, false);
        side.addColour (0.5, black.withAlpha (0.0f));
        g.setGradientFill (side);
        g.fillPath (cp);
    }

    g.setColour (white.withAlpha (0.45f));
    g.fillRect (cap.getX() + capW * 0.12f, cap.getY() + 0.5f, cap.getWidth() - capW * 0.24f, 1.5f);

    g.setColour (red.withAlpha (0.35f));
    g.fillRect (cap.getX() + capW * 0.08f, cap.getCentreY() - 2.5f, capW * 0.84f, 5.0f);
    g.setColour (red.brighter (0.2f));
    g.fillRect (cap.getX() + capW * 0.08f, cap.getCentreY() - 1.0f, capW * 0.84f, 2.0f);

    g.setColour (white.withAlpha (0.45f));
    g.drawRoundedRectangle (cap, capW * 0.12f, 1.0f);

    g.endTransparencyLayer();
}

void ZatLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
    bool over, bool down)
{
    const auto style = button.getProperties()["zatStyle"].toString();

    if (style == "lamp") drawLamp (g, button, over, down);
    else if (style == "auto") drawAutoButton (g, button, over);
    else LookAndFeel_V4::drawToggleButton (g, button, over, down);
}

void ZatLookAndFeel::drawLamp (juce::Graphics& g, juce::ToggleButton& btn, bool over, bool down)
{
    const auto b = btn.getLocalBounds().toFloat();
    const auto c = b.getCentre();
    const float R = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f * 0.58f;
    const bool on = btn.getToggleState();

    if (on)
    {
        const float gr = R * 1.7f;
        g.setGradientFill (juce::ColourGradient (blue.withAlpha (0.30f), c.x, c.y,
            blue.withAlpha (0.0f), c.x + gr, c.y, true));
        g.fillEllipse (c.x - gr, c.y - gr, gr * 2.f, gr * 2.f);
    }

    juce::Path bez;
    bez.addEllipse (c.x - R, c.y - R, R * 2.f, R * 2.f);
    juce::DropShadow (black.withAlpha (0.8f), juce::roundToInt (R * 0.5f),
        { 0, juce::roundToInt (R * 0.18f) }).drawForPath (g, bez);
    g.setGradientFill (juce::ColourGradient (platinum.brighter (over ? 0.1f : 0.0f), c.x, c.y - R,
        platinum.darker (1.8f), c.x, c.y + R, false));
    g.fillPath (bez);

    const float rr = R * 0.8f;
    g.setGradientFill (juce::ColourGradient (onyx, c.x, c.y - rr, platinum.darker (1.0f), c.x, c.y + rr, false));
    g.fillEllipse (c.x - rr, c.y - rr, rr * 2.f, rr * 2.f);

    const float lr = R * 0.68f;
    if (on)
        g.setGradientFill (juce::ColourGradient (blue.brighter (0.6f), c.x - lr * 0.2f, c.y - lr * 0.25f,
            blue.darker (0.1f), c.x + lr, c.y + lr, true));
    else
        g.setGradientFill (juce::ColourGradient (blue.darker (1.4f), c.x, c.y - lr * 0.2f,
            onyx, c.x + lr, c.y, true));
    g.fillEllipse (c.x - lr, c.y - lr, lr * 2.f, lr * 2.f);

    juce::Rectangle<float> sp (lr * 1.0f, lr * 0.55f);
    sp.setCentre (c.x - lr * 0.15f, c.y - lr * 0.45f);
    g.setGradientFill (juce::ColourGradient (white.withAlpha (on ? 0.55f : 0.28f), 0.f, sp.getY(),
        white.withAlpha (0.0f), 0.f, sp.getBottom(), false));
    g.fillEllipse (sp);

    if (down)
    {
        g.setColour (black.withAlpha (0.2f));
        g.fillEllipse (c.x - lr, c.y - lr, lr * 2.f, lr * 2.f);
    }
}

void ZatLookAndFeel::drawAutoButton (juce::Graphics& g, juce::ToggleButton& btn, bool over)
{
    const auto b = btn.getLocalBounds().toFloat().reduced (1.0f);
    const float h = b.getHeight();
    const bool on = btn.getToggleState();

    g.setColour (onyx);
    g.fillRoundedRectangle (b, h * 0.5f);
    g.setColour (on ? red.withAlpha (0.85f) : white.withAlpha (over ? 0.3f : 0.15f));
    g.drawRoundedRectangle (b, h * 0.5f, 1.2f);

    const juce::Point<float> lc (b.getX() + h * 0.55f, b.getCentreY());
    if (on)
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (red.withAlpha (0.12f));
            const float rr = h * 0.17f + (float) i * h * 0.1f;
            g.fillEllipse (lc.x - rr, lc.y - rr, rr * 2.f, rr * 2.f);
        }
    g.setColour (on ? red.brighter (0.3f) : cyan.darker (1.2f));
    g.fillEllipse (lc.x - h * 0.17f, lc.y - h * 0.17f, h * 0.34f, h * 0.34f);

    g.setFont (makeFont (h * 0.46f));
    g.setColour (on ? white : textCol.withAlpha (0.75f));
    g.drawText ("AUTO", b.withTrimmedLeft (h * 0.9f), juce::Justification::centred);
}