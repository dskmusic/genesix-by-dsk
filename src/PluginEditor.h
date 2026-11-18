#pragma once
#include "PluginProcessor.h"

class GenesixEditor : public juce::AudioProcessorEditor
{
public:
    explicit GenesixEditor (GenesixProcessor&);
    void paint (juce::Graphics&) override;

private:
    GenesixProcessor& proc;
};
