#pragma once

#include <JuceHeader.h>
#include "../Styles/Palette.h"

class GainStrip : public juce::Component
{
public:
    GainStrip (juce::AudioProcessorValueTreeState& apvts, const juce::String& titleText,
        const juce::String& gainID, bool meterOnRightSide,
        const juce::String& autoID = {}, bool showLevelMeter = false)
        : title (titleText),
          meterOnRight (meterOnRightSide),
          showMeter (showLevelMeter),
          attachment (apvts, gainID, fader)
    {
        fader.setSliderStyle (juce::Slider::LinearVertical);
        fader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        fader.setSliderSnapsToMousePosition (false);
        fader.setDoubleClickReturnValue (true, 0.0);
        fader.onValueChange = [this] { repaint (readoutArea); };
        addAndMakeVisible (fader);

        if (autoID.isNotEmpty())
        {
            autoButton = std::make_unique<juce::ToggleButton>();
            autoButton->getProperties().set ("zatStyle", "auto");
            autoButton->setMouseCursor (juce::MouseCursor::PointingHandCursor);
            addAndMakeVisible (*autoButton);

            autoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, autoID, *autoButton);
            autoButton->onStateChange = [this] { fader.setEnabled (! autoButton->getToggleState()); };
            fader.setEnabled (! autoButton->getToggleState());
        }
    }

    void pushLevel (float peakLinear)
    {
        if (! showMeter) return;
        const float db = juce::Decibels::gainToDecibels (peakLinear, -60.0f);
        levelDb = (db > levelDb) ? db : juce::jmax (db, levelDb - 2.5f);
        repaint (meterArea.getSmallestIntegerContainer());
    }

    void resized() override
    {
        const float s = (float) getWidth() / 84.0f;
        auto px = [s] (float v) { return juce::roundToInt (v * s); };

        auto b = getLocalBounds();
        titleArea = b.removeFromTop (px (28));
        auto bottom = b.removeFromBottom (px (36));
        readoutArea = b.removeFromBottom (px (24));
        auto mid = b.reduced (0, px (6));

        const int faderW = px (66);

        if (! showMeter)
        {
            fader.setBounds (mid.withSizeKeepingCentre (faderW, mid.getHeight()));
        }
        else
        {
            const int meterW = px (10), gap = px (4);
            auto row = mid.withSizeKeepingCentre (meterW + gap + faderW, mid.getHeight());
            if (meterOnRight)
            {
                fader.setBounds (row.removeFromLeft (faderW));
                row.removeFromLeft (gap);
                meterArea = row.toFloat();
            }
            else
            {
                meterArea = row.removeFromLeft (meterW).toFloat();
                row.removeFromLeft (gap);
                fader.setBounds (row);
            }
        }

        if (autoButton != nullptr)
            autoButton->setBounds (bottom.withSizeKeepingCentre (px (64), px (24)));
    }

    void paint (juce::Graphics& g) override
    {
        using namespace ZatUI;

        g.setFont (makeFont ((float) getWidth() * 0.17f));
        g.setColour (black.withAlpha (0.6f));
        g.drawText (title, titleArea.translated (0, 1), juce::Justification::centred);
        g.setColour (textCol);
        g.drawText (title, titleArea, juce::Justification::centred);

        if (showMeter)
        {
            constexpr int N = 28;
            const float segH = meterArea.getHeight() / (float) N;
            const float pos = juce::jlimit (0.0f, 1.0f, (levelDb + 48.0f) / 54.0f);
            for (int i = 0; i < N; ++i)
            {
                const float t = (float) i / (float) N;
                const juce::Rectangle<float> seg (meterArea.getX(), meterArea.getBottom() - (float) (i + 1) * segH + 1.0f,
                    meterArea.getWidth(), segH - 2.0f);
                const float segDb = -48.0f + t * 54.0f;
                const auto col = segDb > -3.0f ? hot : (segDb > -12.0f ? red : cyan);
                g.setColour (t < pos ? col : white.withAlpha (0.06f));
                g.fillRoundedRectangle (seg, 1.0f);
            }
        }

        g.setFont (makeFont ((float) getWidth() * 0.15f));
        g.setColour (textCol.withAlpha (fader.isEnabled() ? 0.9f : 0.55f));
        g.drawText (juce::String (fader.getValue(), 1) + " dB", readoutArea, juce::Justification::centred);
    }

private:
    juce::String title;
    bool meterOnRight;
    bool showMeter;
    juce::Slider fader;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    std::unique_ptr<juce::ToggleButton> autoButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoAttachment;

    juce::Rectangle<int> titleArea, readoutArea;
    juce::Rectangle<float> meterArea;
    float levelDb = -60.0f;
};