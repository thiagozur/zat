#include "PluginEditor.h"

ZatAudioProcessorEditor::ZatAudioProcessorEditor (ZatAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      typeSelector (p.apvts, ParameterIDs::satType),
      amountAttachment (p.apvts, ParameterIDs::satAmount, amountKnob),
      weight (p.apvts, "weight", ParameterIDs::weightEnable, ParameterIDs::weightAmount),
      air (p.apvts, "air", ParameterIDs::airEnable, ParameterIDs::airAmount),
      range (p.apvts, ParameterIDs::freqLowCut, ParameterIDs::freqHighCut),
      inStrip (p.apvts, "IN", ParameterIDs::inputGain, true),
      outStrip (p.apvts, "OUT", ParameterIDs::outputGain, false, ParameterIDs::autoGain)
{
    setLookAndFeel (&laf);

    addAndMakeVisible (typeSelector);
    addAndMakeVisible (amountKnob);
    addAndMakeVisible (weight);
    addAndMakeVisible (air);
    addAndMakeVisible (range);
    addAndMakeVisible (inStrip);
    addAndMakeVisible (outStrip);

    amountKnob.setDoubleClickReturnValue (true, 20.0);
    amountKnob.onValueChange = [this] { repaint (amountLabelArea); };

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio (1024.0 / 658.0);
    setResizeLimits (640, 411, 1536, 987);
    setSize (900, 578);

    startTimerHz (30);
}

ZatAudioProcessorEditor::~ZatAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void ZatAudioProcessorEditor::timerCallback()
{
    const float inPeak = audioProcessor.inputPeak.exchange (0.0f);
    const float outPeak = audioProcessor.outputPeak.exchange (0.0f);

    inStrip.pushLevel (inPeak);
    outStrip.pushLevel (outPeak);

    const float db = juce::Decibels::gainToDecibels (outPeak, -60.0f);
    const float levelNorm = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -48.0f, 0.0f, 0.0f, 1.0f));
    const float amount = (float) amountKnob.getValue() / 100.0f;
    const float target = (0.15f + 0.85f * levelNorm) * (0.3f + 0.7f * amount);

    const float coeff = target > glow ? 0.5f : 0.12f;
    glow += (target - glow) * coeff;
    amountKnob.setGlow (glow);
}

void ZatAudioProcessorEditor::rebuildBackground()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;

    background = juce::Image (juce::Image::RGB, w, h, true);
    juce::Graphics g (background);

    g.setGradientFill (juce::ColourGradient (ZatUI::onyx.interpolatedWith (ZatUI::platinum, 0.10f), 0.f, 0.f,
        ZatUI::onyx, 0.f, (float) h, false));
    g.fillAll();

    g.setGradientFill (juce::ColourGradient (ZatUI::white.withAlpha (0.04f), (float) w * 0.5f, (float) h * 0.45f,
        ZatUI::white.withAlpha (0.0f), (float) w * 1.05f, (float) h * 0.45f, true));
    g.fillAll();

    juce::Random rnd (7);
    for (int y = 0; y < h; ++y)
    {
        g.setColour ((rnd.nextBool() ? ZatUI::white : ZatUI::black).withAlpha (rnd.nextFloat() * 0.011f));
        g.fillRect (0, y, w, 1);
    }
}

void ZatAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (background, 0, 0);

    const float W = static_cast<float> (getWidth());
    const float H = static_cast<float> (getHeight());

    juce::Rectangle<int> logoArea (
        juce::roundToInt (32.0f / 1024.0f * W),
        juce::roundToInt (24.0f / 658.0f * H),
        juce::roundToInt (120.0f / 1024.0f * W), 
        juce::roundToInt (40.0f / 658.0f * H)    
    );

    juce::Font font (ZatUI::makeFont (H * 0.045f, true));
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (font, "zat", 0.0f, 0.0f);
    juce::Path textPath;
    glyphs.createPath (textPath);

    juce::Rectangle<float> textBounds = textPath.getBounds();
    float xOffset = static_cast<float>(logoArea.getX());
    float yOffset = static_cast<float>(logoArea.getCentreY() - textBounds.getCentreY());
    textPath.applyTransform (juce::AffineTransform::translation (xOffset, yOffset));

    float bevelOffset = 1.0f;

    g.setColour (juce::Colours::black.withAlpha(0.8f)); 
    g.fillPath (textPath, juce::AffineTransform::translation (-bevelOffset, -bevelOffset));

    g.setColour (juce::Colours::white);
    g.fillPath (textPath, juce::AffineTransform::translation (bevelOffset, bevelOffset));

    g.setColour (juce::Colour (0xffefefef));
    g.fillPath (textPath);
    
    g.setColour (ZatUI::textCol);
    g.setFont (ZatUI::makeFont (H * 0.034f));
    g.drawText ("amount", amountLabelArea.withTrimmedBottom (amountLabelArea.getHeight() / 2), juce::Justification::centred);
    g.setColour (ZatUI::textCol.withAlpha (0.85f));
    g.setFont (ZatUI::makeFont (H * 0.032f));
    g.drawText (juce::String (juce::roundToInt (static_cast<float> (amountKnob.getValue()))) + "%",
        amountLabelArea.withTrimmedTop (amountLabelArea.getHeight() / 2), juce::Justification::centred);
}

void ZatAudioProcessorEditor::resized()
{
    const float W = (float) getWidth(), H = (float) getHeight();

    auto R = [W, H] (float x, float y, float w, float h)
    {
        return juce::Rectangle<int> (juce::roundToInt (x / 1024.f * W), juce::roundToInt (y / 658.f * H),
            juce::roundToInt (w / 1024.f * W), juce::roundToInt (h / 658.f * H));
    };

    typeSelector.setBounds (R (352, 20, 320, 70));
    amountKnob.setBounds (R (330, 130, 350, 350));
    amountLabelArea = R (400, 478, 210, 52);

    weight.setBounds (R (130, 262, 164, 250));
    air.setBounds (R (728, 262, 164, 250));

    inStrip.setBounds (R (12, 120, 84, 390));
    outStrip.setBounds (R (928, 120, 84, 390));

    range.setBounds (R (70, 530, 884, 120));

    rebuildBackground();
}