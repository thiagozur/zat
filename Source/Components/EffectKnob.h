#pragma once

#include <JuceHeader.h>
#include "../Styles/Palette.h"

class EffectKnob : public juce::Component
{
public:
    EffectKnob (juce::AudioProcessorValueTreeState& apvts, const juce::String& titleText,
        const juce::String& enableID, const juce::String& amountID)
        : title (titleText),
          knobAttach (apvts, amountID, knob),
          lampAttach (apvts, enableID, lamp)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob.setRotaryParameters (ZatUI::arcStart, ZatUI::arcEnd, true);
        knob.setMouseDragSensitivity (200);
        knob.setDoubleClickReturnValue (true, 50.0);
        knob.getProperties().set ("zatStyle", "mini");
        knob.onValueChange = [this] { repaint (labelArea); };

        lamp.getProperties().set ("zatStyle", "lamp");
        lamp.setMouseCursor (juce::MouseCursor::PointingHandCursor);
        lamp.setTooltip ("Click to turn " + titleText + " on / off");
        lamp.onStateChange = [this] { repaint (stateArea); };

        addAndMakeVisible (knob);
        addAndMakeVisible (lamp);
    }

    void resized() override
    {
        const int w = getWidth(), h = getHeight();

        stateArea = { 0, 0, w, juce::roundToInt ((float) h * 0.08f) };

        const int ls = juce::roundToInt ((float) h * 0.27f);
        lamp.setBounds (juce::Rectangle<int> (ls, ls).withCentre ({ w / 2, juce::roundToInt ((float) h * 0.225f) }));

        const int ks = juce::jmin (w, juce::roundToInt ((float) h * 0.44f));
        knob.setBounds (juce::Rectangle<int> (ks, ks).withCentre ({ w / 2, juce::roundToInt ((float) h * 0.59f) }));

        const int ly = juce::roundToInt ((float) h * 0.81f);
        labelArea = { 0, ly, w, h - ly };
    }

    void paint (juce::Graphics& g) override
    {
        using namespace ZatUI;
        const float h = (float) getHeight();

        const bool on = lamp.getToggleState();
        g.setFont (makeFont (h * 0.066f));
        g.setColour (on ? blue.brighter (0.6f) : blue.brighter (0.6f).darker (0.9f));
        g.drawText (on ? "ON" : "OFF", stateArea, juce::Justification::centred);

        const auto nameArea = labelArea.withHeight (labelArea.getHeight() / 2);
        const auto valueArea = labelArea.withTrimmedTop (labelArea.getHeight() / 2);

        g.setFont (makeFont (h * 0.077f));
        g.setColour (textCol);
        g.drawText (title, nameArea, juce::Justification::centred);

        g.setFont (makeFont (h * 0.073f));
        g.setColour (textCol.withAlpha (0.85f));
        g.drawText (juce::String (juce::roundToInt ((float) knob.getValue())) + "%", valueArea, juce::Justification::centred);
    }

private:
    juce::String title;
    juce::Slider knob;
    juce::ToggleButton lamp;
    juce::AudioProcessorValueTreeState::SliderAttachment knobAttach;
    juce::AudioProcessorValueTreeState::ButtonAttachment lampAttach;
    juce::Rectangle<int> stateArea, labelArea;
};