#include "Tests.h"
#include "PluginEditor.h"
#include "gen/Generator.h"

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
    if (cmd == "gen-debug")
    {
        gx::GenRequest r; r.category = juce::String::fromUTF8 (argv[2]); r.seed = (uint32_t) juce::String (argv[3]).getIntValue(); if (argc > 4) r.subcategory = argv[4];
        for (int k = 0; k < 5; ++k)
        {
            auto p = gx::Generator::candidate (r, r.seed + (uint32_t) k * 7919u);
            const float m0 = p.params[(size_t) gx::P::master];
            auto v = gx::Generator::validate (p);
            std::printf ("%s/%s '%s': ok=%d %s rms=%.1f peak=%.1f dc=%.4f sus=%.2f tail=%.1f cent=%.0f flat=%.2f master %.1f->%.1f\n",
                         p.meta.category.toRawUTF8(), p.meta.subcategory.toRawUTF8(), p.meta.name.toRawUTF8(), (int) v.ok, v.reason.toRawUTF8(),
                         v.rmsDb, v.peakDb, v.dc, v.sustainRatio, v.tailDb, v.centroidHz, v.flatness, m0, p.params[(size_t) gx::P::master]);
        }
        return 0;
    }
    if (cmd == "gen-report")
    {
        // GenesixTool gen-report [count]: attempts and validation per category
        const int count = argc > 2 ? juce::String (argv[2]).getIntValue() : 10;
        for (auto& cat : gx::categories())
        {
            int att = 0, fails = 0; float rms = 0;
            std::map<juce::String, int> reasons;
            for (int i = 0; i < count; ++i)
            {
                gx::GenRequest r; r.category = cat; r.seed = (uint32_t) (1000 + i * 31);
                juce::Random rnd (i);
                r.brightness = rnd.nextFloat(); r.movement = rnd.nextFloat(); r.aggression = rnd.nextFloat(); r.space = rnd.nextFloat(); r.weirdness = rnd.nextFloat();
                for (int k = 0; k < 24; ++k)
                {
                    auto p = gx::Generator::candidate (r, r.seed + (uint32_t) k * 7919u);
                    auto v = gx::Generator::validate (p);
                    ++att;
                    if (v.ok) { rms += v.rmsDb; break; }
                    reasons[v.reason]++;
                    if (k == 23) ++fails;
                }
            }
            juce::String rs;
            for (auto& [k, n] : reasons) rs << k << "=" << n << " ";
            std::printf ("%-11s attempts/preset %.2f  fails %d  rejects: %s\n", cat.toRawUTF8(), (double) att / count, fails, rs.toRawUTF8());
        }
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
