#include "PluginProcessor.h"

static int runTests()
{
    GenesixProcessor p;
    p.prepareToPlay (48000.0, 512);
    juce::AudioBuffer<float> b (2, 512);
    juce::MidiBuffer m;
    p.processBlock (b, m);
    std::puts ("tests: OK");
    return 0;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::String cmd = argc > 1 ? argv[1] : "test";
    if (cmd == "test") return runTests();
    std::fprintf (stderr, "usage: GenesixTool test\n");
    return 2;
}
