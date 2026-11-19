#pragma once
#include "PluginProcessor.h"
#include <functional>

namespace gxtest
{
struct Stats { float peak = 0, rms = 0, dc = 0; bool finite = true; };
Stats analyse (const juce::AudioBuffer<float>& b);
void check (bool ok, const juce::String& what);
void setParam (GenesixProcessor& p, const juce::String& id, float plain);
juce::AudioBuffer<float> renderNote (GenesixProcessor& p, int note, double seconds, double releaseAt, float vel = 0.8f, double sr = 48000.0);
std::vector<std::function<void()>>& extraTests();
int runAll();
}
