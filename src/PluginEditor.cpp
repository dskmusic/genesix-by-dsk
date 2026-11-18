#include "PluginEditor.h"

GenesixEditor::GenesixEditor (GenesixProcessor& p) : AudioProcessorEditor (p), proc (p)
{
    setSize (1200, 760);
}

void GenesixEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e2124));
    g.setColour (juce::Colour (0xffff8a1f));
    g.setFont (juce::FontOptions (48.0f, juce::Font::bold));
    g.drawText ("Genesix", getLocalBounds(), juce::Justification::centred);
}
