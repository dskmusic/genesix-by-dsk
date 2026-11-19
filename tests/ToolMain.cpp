#include "Tests.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::String cmd = argc > 1 ? argv[1] : "test";
    if (cmd == "test") return gxtest::runAll();
    if (cmd == "bench")
    {
        // worst-ish case: 16 voices x 8 unison x 4 oscillators of mixed types, both filters, all FX
        GenesixProcessor p;
        const char* types[] { "1", "0", "2", "4" };
        for (int o = 1; o <= 4; ++o)
        {
            gxtest::setParam (p, "osc" + juce::String (o) + "_on", 1);
            gxtest::setParam (p, "osc" + juce::String (o) + "_type", juce::String (types[o - 1]).getFloatValue());
        }
        gxtest::setParam (p, "uni_voices", 8);
        gxtest::setParam (p, "filter2_on", 1);
        for (auto id : { "chorus_on", "delay_on", "reverb_on", "eq_on" }) gxtest::setParam (p, id, 1);
        const double sr = 48000, secs = 10;
        juce::AudioBuffer<float> out (2, (int) (sr * secs));
        juce::MidiBuffer m;
        for (int i = 0; i < 16; ++i) m.addEvent (juce::MidiMessage::noteOn (1, 40 + i * 2, 0.8f), 0);
        const auto t0 = juce::Time::getMillisecondCounterHiRes();
        p.renderOffline (out, m, sr, 256);
        const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
        std::printf ("rendered %.0f s in %.0f ms -> %.1f%% of one core\n", secs, ms, ms / (secs * 10.0));
        return 0;
    }
    std::fprintf (stderr, "usage: GenesixTool test\n");
    return 2;
}
