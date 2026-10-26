#include "PluginProcessor.h"
#include "PluginEditor.h"

ZatAudioProcessorEditor::ZatAudioProcessorEditor (ZatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

ZatAudioProcessorEditor::~ZatAudioProcessorEditor() {}

void ZatAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ZatAudioProcessorEditor::resized() {}