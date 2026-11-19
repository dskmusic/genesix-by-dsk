#include "Tests.h"
#include "PluginEditor.h"

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
    if (cmd == "screenshot")
    {
        // GenesixTool screenshot <outdir>
        const juce::File out = juce::File::getCurrentWorkingDirectory().getChildFile (argc > 2 ? argv[2] : "docs/screenshots");
        out.createDirectory();
        GenesixProcessor p;
        p.prepareToPlay (48000, 512);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        auto* ge = dynamic_cast<GenesixEditor*> (ed.get());
        ed->setSize (GenesixEditor::kDesignW, GenesixEditor::kDesignH);
        ed->addToDesktop (0);
        ed->setVisible (true);
        juce::AudioBuffer<float> buf (2, 512);
        bool first = true;
        auto pump = [&] (int frames)
        {
            for (int k = 0; k < frames; ++k)
            {
                juce::MidiBuffer m;
                if (first) { for (int n : { 48, 55, 60, 64, 67 }) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.8f), 0); first = false; }
                for (int b = 0; b < 2; ++b) { p.processBlock (buf, m); m.clear(); }
                ge->advanceFrames (1.0 / 60.0);
                juce::MessageManager::getInstance()->runDispatchLoopUntil (4);
            }
        };
        struct Shot { int page, sub; const char* name; };
        const Shot shots[] { { 0, -1, "generator" }, { 1, -1, "presets" }, { 2, 0, "editor-osc" }, { 2, 1, "editor-filters" },
                             { 2, 2, "editor-mod" }, { 2, 3, "editor-arp-gate-seq" }, { 2, 4, "editor-fx" }, { 3, -1, "export" }, { 4, -1, "settings" } };
        for (auto& s : shots)
        {
            ge->showPage (s.page);
            if (s.sub >= 0) ge->showEditorSub (s.sub);
            pump (40);
            auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            juce::File f = out.getChildFile (juce::String (s.name) + ".png");
            f.deleteFile();
            juce::FileOutputStream os (f);
            juce::PNGImageFormat().writeImageToStream (img, os);
            std::printf ("wrote %s\n", f.getFullPathName().toRawUTF8());
        }
        ed->removeFromDesktop();
        ed.reset();
        return 0;
    }
    std::fprintf (stderr, "usage: GenesixTool test\n");
    return 2;
}
