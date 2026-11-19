// GenesixTool video <outdir>
// Scripted ~70 s product demo: renders the real editor offscreen at 30 fps on a virtual clock,
// drives it like a user (animated cursor, clicks, slider drags, typing), plays example phrases
// through the real engine, and writes frames/f%05d.jpg + audio.wav (mux with ffmpeg).
#include "PluginEditor.h"
#include "ui/GeneratorPage.h"
#include "gen/Generator.h"
#include <map>

namespace
{
constexpr int kFps = 30, kW = 1920, kH = 1080, kSub = 8;
constexpr double kSr = 48000.0;
constexpr int kSamplesPerFrame = (int) (kSr / kFps), kBlock = kSamplesPerFrame / kSub;
constexpr float kUiScale = 1.35f;                       // 1280x800 design -> 1728x1080
const int kUiX = (kW - (int) (1280 * kUiScale)) / 2;

using namespace gx::ui;

struct Note { double beat; int note; double len; float vel; };

class Director
{
public:
    Director (const juce::File& dir) : out (dir)
    {
        out.getChildFile ("frames").createDirectory();
        virtualClock() = [this] { return vt; };
        forcedSeed() = 7001;
        gx::forcedLanguage() = "en";
        proc.prepareToPlay (kSr, kBlock);
        ed.reset (dynamic_cast<GenesixEditor*> (proc.createEditor()));
        ed->setSize (GenesixEditor::kDesignW, GenesixEditor::kDesignH);
        ed->addToDesktop (0);
        ed->setVisible (true);
        cursor = { 900.0f, 520.0f };
    }

    ~Director()
    {
        ed->removeFromDesktop();
        ed.reset();
        virtualClock() = nullptr;
    }

    // ------------------------------------------------------------ primitives
    void frame()
    {
        vt += 1.0 / kFps;
        juce::AudioBuffer<float> buf (2, kBlock);
        for (int s = 0; s < kSub; ++s)
        {
            while (! events.empty() && events.begin()->first <= sample)
            {
                auto [n, on, vel] = events.begin()->second;
                if (on) proc.keyboardState.noteOn (1, n, vel); else proc.keyboardState.noteOff (1, n, 0.0f);
                events.erase (events.begin());
            }
            juce::MidiBuffer m;
            buf.clear();
            proc.processBlock (buf, m);
            for (int i = 0; i < kBlock; ++i) { L.push_back (buf.getSample (0, i)); R.push_back (buf.getSample (1, i)); }
            sample += kBlock;
        }
        ed->videoFrame();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (3);
        writeFrame();
    }
    void wait (double sec) { for (int i = 0, n = (int) std::lround (sec * kFps); i < n; ++i) frame(); }

    void play (const std::vector<Note>& ph, double bpm = 120.0, int transpose = 0)
    {
        const double spb = 60.0 / bpm * kSr;
        for (auto& nt : ph)
        {
            const auto t0 = sample + (juce::int64) (nt.beat * spb), t1 = sample + (juce::int64) ((nt.beat + nt.len) * spb);
            events.insert ({ t0, { nt.note + transpose, true, nt.vel } });
            events.insert ({ t1, { nt.note + transpose, false, 0.0f } });
        }
    }
    void silence() { for (int n = 0; n < 128; ++n) proc.keyboardState.noteOff (1, n, 0.0f); events.clear(); }

    void caption (const char* title, const char* sub = "") { capTitle = juce::String::fromUTF8 (title); capSub = juce::String::fromUTF8 (sub); capStart = vt; }
    void clearCaption() { capEnd = vt; }

    juce::Point<float> centreOf (juce::Component* c) const
    {
        return ed->getLocalArea (c, c->getLocalBounds()).toFloat().getCentre();
    }
    void moveTo (juce::Point<float> p, double sec = 0.55)
    {
        const auto from = cursor;
        const int n = juce::jmax (1, (int) std::lround (sec * kFps));
        for (int i = 1; i <= n; ++i)
        {
            const float t = (float) i / (float) n, e = t * t * (3.0f - 2.0f * t);
            cursor = from + (p - from) * e;
            frame();
        }
    }
    void moveTo (juce::Component* c, double sec = 0.55) { if (c) moveTo (centreOf (c), sec); }
    void clickFx() { clickAt = vt; clickPos = cursor; }

    void click (juce::Component* c, double settle = 0.35)
    {
        if (c == nullptr) { std::printf ("video: target not found\n"); return; }
        moveTo (c);
        clickFx();
        if (auto* b = dynamic_cast<juce::Button*> (c)) b->triggerClick();
        else if (auto* g = dynamic_cast<GenerateButton*> (c)) { if (g->onClick) g->onClick(); }
        wait (settle);
    }

    void drag (juce::Slider* s, double to, double sec = 0.9)
    {
        if (s == nullptr) return;
        auto thumb = [&] (double v)
        {
            const auto b = ed->getLocalArea (s, s->getLocalBounds()).toFloat();
            const float pos = (float) s->getPositionOfValue (v);
            return s->isVertical() ? juce::Point<float> (b.getCentreX(), b.getY() + pos) : juce::Point<float> (b.getX() + pos, b.getCentreY());
        };
        moveTo (thumb (s->getValue()), 0.45);
        clickFx();
        const double from = s->getValue();
        const int n = (int) std::lround (sec * kFps);
        for (int i = 1; i <= n; ++i)
        {
            const double t = (double) i / n, e = t * t * (3 - 2 * t);
            s->setValue (from + (to - from) * e, juce::sendNotificationSync);
            cursor = thumb (s->getValue());
            frame();
        }
    }

    void type (juce::TextEditor* te, const juce::String& text)
    {
        if (te == nullptr) return;
        click (te, 0.1);
        for (int i = 1; i <= text.length(); ++i) { te->setText (text.substring (0, i), true); wait (0.12); }
    }

    void selectRow (juce::ListBox* lb, int row)
    {
        if (lb == nullptr) return;
        const auto r = lb->getRowPosition (row, true).toFloat();
        moveTo (ed->getLocalArea (lb, r.toNearestInt()).toFloat().getCentre().translated (-150.0f, 0.0f));
        clickFx();
        lb->selectRow (row);
        wait (0.3);
    }

    // generate with the big button and wait until the new sound is in (plus the assemble animation)
    void generate()
    {
        click (find<GenerateButton> (&ed->generatorPage()), 0.1);
        for (int i = 0; i < 12 * kFps && ed->generatorPage().isBusy(); ++i) frame();
        wait (0.5);
    }

    // ------------------------------------------------------------ finders (visible components only)
    template <typename T>
    T* find (juce::Component* root, std::function<bool (T*)> pred = {}) const
    {
        if (root == nullptr || ! root->isShowing()) return nullptr;
        if (auto* t = dynamic_cast<T*> (root)) if (! pred || pred (t)) return t;
        for (auto* ch : root->getChildren()) if (auto* r = find<T> (ch, pred)) return r;
        return nullptr;
    }
    juce::Button* button (const juce::String& text, juce::Component* root = nullptr) const
    {
        return find<juce::Button> (root ? root : ed.get(), [&] (juce::Button* b) { return b->getButtonText() == text; });
    }
    juce::Slider* genSlider (int index) const
    {
        int k = 0;
        return find<juce::Slider> (&ed->generatorPage(), [&] (juce::Slider* s) { return s->getSliderStyle() == juce::Slider::LinearVertical && k++ == index; });
    }

    GenesixProcessor proc;
    std::unique_ptr<GenesixEditor> ed;

    // ------------------------------------------------------------ overlays: title cards
    double cardFrom = -1, cardTo = -1; bool endCard = false;
    void card (bool isEnd, double fadeIn, double hold, double fadeOut)
    {
        endCard = isEnd; cardFrom = vt; cardTo = vt + fadeIn + hold;
        cardFadeIn = fadeIn; cardFadeOut = fadeOut;
        wait (fadeIn + hold);
        cardOutStart = vt;
        wait (fadeOut);
        cardFrom = cardTo = -1;
    }

    void finishAudio()
    {
        // level-match the different sounds: slow AGC towards -17 dBFS RMS, then a soft limiter under -0.5 dBFS
        const size_t hop = (size_t) (kSr * 0.01), win = (size_t) (kSr * 0.3);
        std::vector<float> gainDb (L.size() / hop + 2, 0.0f);
        float gDb = 0.0f;
        for (size_t h = 0; h < gainDb.size(); ++h)
        {
            const size_t c = h * hop, a0 = c > win / 2 ? c - win / 2 : 0, a1 = juce::jmin (L.size(), c + win / 2);
            double e = 0;
            for (size_t i = a0; i < a1; ++i) e += 0.5 * (L[i] * L[i] + R[i] * R[i]);
            const float rmsDb = a1 > a0 ? (float) juce::Decibels::gainToDecibels (std::sqrt (e / (double) (a1 - a0)), -120.0) : -120.0f;
            if (rmsDb > -55.0f) gDb += (juce::jlimit (-24.0f, 24.0f, -17.0f - rmsDb) - gDb) * 0.035f;   // ~0.3 s smoothing
            gainDb[h] = gDb;
        }
        auto limit = [] (float x) { const float a = std::abs (x); return a < 0.7f ? x : std::copysign (0.7f + 0.24f * std::tanh ((a - 0.7f) / 0.24f), x); };
        const size_t fade = (size_t) (kSr * 0.02), fadeEnd = (size_t) (kSr * 1.5);
        juce::AudioBuffer<float> b (2, (int) L.size());
        for (size_t i = 0; i < L.size(); ++i)
        {
            const size_t h = i / hop;
            const float t = (float) (i % hop) / (float) hop;
            float a = juce::Decibels::decibelsToGain (gainDb[h] + (gainDb[h + 1] - gainDb[h]) * t);
            if (i < fade) a *= (float) i / (float) fade;
            if (i + fadeEnd > L.size()) a *= (float) (L.size() - i) / (float) fadeEnd;
            b.setSample (0, (int) i, limit (L[i] * a));
            b.setSample (1, (int) i, limit (R[i] * a));
        }
        auto f = out.getChildFile ("audio.wav");
        f.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> os (f.createOutputStream().release());
        auto w = wav.createWriterFor (os, juce::AudioFormatWriterOptions().withSampleRate (kSr).withNumChannels (2).withBitsPerSample (24));
        if (w) w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
        std::printf ("audio: %.1f s, peak gain %+.1f dB\n", (double) L.size() / kSr, 0.0);
    }

private:
    void writeFrame()
    {
        juce::Image img (juce::Image::RGB, kW, kH, true);
        {   // scope: native (Direct2D) contexts only flush when the Graphics is destroyed
            juce::Graphics g (img);
            g.fillAll (col::bgDeep);
            auto ui = ed->createComponentSnapshot (ed->getLocalBounds(), true, kUiScale);
            g.drawImageAt (ui, kUiX, 0);
            drawCaption (g);
            drawCursor (g);
            drawCard (g);
        }
        auto f = out.getChildFile ("frames").getChildFile (juce::String::formatted ("f%05d.jpg", frameNo++));
        f.deleteFile();
        juce::FileOutputStream os (f);
        juce::JPEGImageFormat jpg;
        jpg.setQuality (0.94f);
        jpg.writeImageToStream (img, os);
    }

    juce::Point<float> toScreen (juce::Point<float> p) const { return { kUiX + p.x * kUiScale, p.y * kUiScale }; }

    void drawCursor (juce::Graphics& g)
    {
        if (cardFrom >= 0) return;
        const auto p = toScreen (cursor);
        const double ca = vt - clickAt;
        if (ca >= 0 && ca < 0.45)
        {
            const float t = (float) (ca / 0.45);
            const auto c = toScreen (clickPos);
            g.setColour (col::accent.withAlpha (0.9f * (1.0f - t)));
            g.drawEllipse (juce::Rectangle<float> (10 + 46 * t, 10 + 46 * t).withCentre (c), 3.0f);
        }
        juce::Path a;
        a.startNewSubPath (0, 0); a.lineTo (0, 26); a.lineTo (7, 20); a.lineTo (12, 31); a.lineTo (17, 29);
        a.lineTo (12, 18); a.lineTo (21, 18); a.closeSubPath();
        a.applyTransform (juce::AffineTransform::scale (1.15f).translated (p));
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillPath (a, juce::AffineTransform::translation (2, 3));
        g.setColour (juce::Colours::white);
        g.fillPath (a);
        g.setColour (juce::Colours::black);
        g.strokePath (a, juce::PathStrokeType (1.6f));
    }

    static float textWidth (const juce::Font& f, const juce::String& t)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (f, t, 0, 0);
        return ga.getBoundingBox (0, -1, true).getWidth();
    }

    void drawCaption (juce::Graphics& g)
    {
        if (capTitle.isEmpty()) return;
        float a = (float) juce::jlimit (0.0, 1.0, (vt - capStart) / 0.3);
        if (capEnd > capStart) a *= (float) juce::jlimit (0.0, 1.0, 1.0 - (vt - capEnd) / 0.3);
        if (a <= 0.0f) return;
        auto big = font (34.0f, true), small = font (22.0f);
        const float w = juce::jmax (textWidth (big, capTitle), textWidth (small, capSub)) + 70.0f;
        const float h = capSub.isEmpty() ? 64.0f : 100.0f;
        juce::Rectangle<float> r ((kW - w) * 0.5f, kH - 6.0f - h, w, h);
        g.setColour (juce::Colour (0xf0101214).withMultipliedAlpha (a));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (col::accent.withAlpha (a));
        g.fillRoundedRectangle (r.withWidth (6.0f), 3.0f);
        g.setColour (col::text.withAlpha (a));
        g.setFont (big);
        g.drawText (capTitle, r.withTrimmedLeft (10).withHeight (64.0f).translated (0, capSub.isEmpty() ? 0.0f : 6.0f), juce::Justification::centred);
        if (capSub.isNotEmpty())
        {
            g.setColour (col::textDim.withAlpha (a));
            g.setFont (small);
            g.drawText (capSub, r.withTrimmedLeft (10).withTrimmedTop (58).withHeight (30.0f), juce::Justification::centred);
        }
    }

    void drawCard (juce::Graphics& g)
    {
        if (cardFrom < 0) return;
        float a = cardFadeIn <= 0 ? 1.0f : (float) juce::jlimit (0.0, 1.0, (vt - cardFrom) / cardFadeIn);
        if (vt > cardTo) a = (float) juce::jlimit (0.0, 1.0, 1.0 - (vt - cardOutStart) / juce::jmax (0.01, cardFadeOut));
        if (a <= 0.0f) return;
        juce::Graphics::ScopedSaveState ss (g);
        g.setOpacity (a);
        juce::Image layer (juce::Image::ARGB, kW, kH, true);
        {
            juce::Graphics lg (layer);
            lg.setGradientFill (juce::ColourGradient (col::bg, kW * 0.5f, kH * 0.4f, col::bgDeep, 0, kH, true));
            lg.fillAll();
            const float t = (float) (vt - cardFrom);
            {
                juce::Graphics::ScopedSaveState s2 (lg);
                lg.addTransform (juce::AffineTransform::scale (4.2f).translated (kW * 0.5f - 32 * 4.2f, (endCard ? 250.0f : 300.0f) - 32 * 4.2f));
                Logo::drawMark (lg, { 0, 0, 64, 64 }, 0.6f + 0.4f * std::sin (t * 2.0f));
            }
            lg.setColour (col::accent);
            lg.setFont (font (110.0f, true));
            lg.drawText ("Genesix", 0, endCard ? 410 : 460, kW, 120, juce::Justification::centred);
            lg.setColour (col::text);
            lg.setFont (font (34.0f, true));
            lg.drawText ("by DSK Music", 0, endCard ? 530 : 580, kW, 44, juce::Justification::centred);
            lg.setColour (col::textDim);
            lg.setFont (font (28.0f));
            if (! endCard)
                lg.drawText ("Free software synthesizer with a smart preset generator", 0, 660, kW, 40, juce::Justification::centred);
            else
            {
                lg.drawText ("Free & open source (GPLv3)", 0, 610, kW, 40, juce::Justification::centred);
                lg.drawText (juce::String::fromUTF8 ("VST3 \xc2\xb7 AU \xc2\xb7 Standalone   \xe2\x80\x94   Windows \xc2\xb7 macOS \xc2\xb7 Linux"), 0, 656, kW, 40, juce::Justification::centred);
                lg.setColour (col::accent2);
                lg.setFont (font (40.0f, true));
                lg.drawText ("dskmusic.com", 0, 740, kW, 50, juce::Justification::centred);
            }
        }
        g.drawImageAt (layer, 0, 0);
    }

    juce::File out;
    double vt = 1000.0;                 // virtual clock (s)
    juce::int64 sample = 0;
    int frameNo = 0;
    std::multimap<juce::int64, std::tuple<int, bool, float>> events;
    std::vector<float> L, R;
    juce::Point<float> cursor, clickPos;
    double clickAt = -10;
    juce::String capTitle, capSub;
    double capStart = 0, capEnd = 0;
    double cardFadeIn = 0, cardFadeOut = 0, cardOutStart = 0;
};

// ---------------------------------------------------------------- music (A minor, 120 bpm)
const std::vector<Note> bassRiff {
    { 0, 33, 0.45, 0.95f }, { 0.5, 33, 0.3, 0.7f }, { 1, 45, 0.4, 0.85f }, { 1.5, 33, 0.3, 0.7f },
    { 2, 36, 0.45, 0.9f }, { 2.5, 33, 0.3, 0.7f }, { 3, 31, 0.45, 0.9f }, { 3.5, 43, 0.3, 0.75f },
    { 4, 29, 0.45, 0.95f }, { 4.5, 29, 0.3, 0.7f }, { 5, 41, 0.4, 0.85f }, { 5.5, 29, 0.3, 0.7f },
    { 6, 31, 0.45, 0.9f }, { 6.5, 31, 0.3, 0.7f }, { 7, 43, 0.4, 0.85f }, { 7.5, 40, 0.45, 0.8f } };
const std::vector<Note> padChords {
    { 0, 45, 3.8, 0.75f }, { 0, 57, 3.8, 0.7f }, { 0, 60, 3.8, 0.7f }, { 0, 64, 3.8, 0.7f }, { 0, 71, 3.8, 0.6f },
    { 4, 41, 3.8, 0.75f }, { 4, 57, 3.8, 0.7f }, { 4, 60, 3.8, 0.7f }, { 4, 64, 3.8, 0.7f }, { 4, 69, 3.8, 0.6f } };
const std::vector<Note> leadMelody {
    { 0, 76, 0.9, 0.9f }, { 1, 74, 0.45, 0.8f }, { 1.5, 72, 0.45, 0.8f }, { 2, 74, 0.9, 0.85f }, { 3, 69, 0.9, 0.8f },
    { 4, 72, 0.45, 0.85f }, { 4.5, 74, 0.45, 0.85f }, { 5, 76, 0.9, 0.9f }, { 6, 79, 0.45, 0.9f }, { 6.5, 76, 1.4, 0.85f } };
const std::vector<Note> leadShort {
    { 0, 76, 0.45, 0.9f }, { 0.5, 79, 0.45, 0.85f }, { 1, 81, 0.9, 0.9f }, { 2, 79, 0.45, 0.8f }, { 2.5, 76, 1.3, 0.85f } };
std::vector<Note> heldChords (int count)   // Am F C G ... held 4 s each (arp presets arpeggiate them)
{
    const int ch[4][4] { { 45, 57, 60, 64 }, { 41, 57, 60, 65 }, { 48, 55, 60, 64 }, { 43, 55, 59, 62 } };
    std::vector<Note> v;
    for (int c = 0; c < count; ++c)
        for (int k = 0; k < 4; ++k) v.push_back ({ c * 8.0, ch[c % 4][k], 7.7, k == 0 ? 0.85f : 0.8f });
    return v;
}
std::vector<Note> pluckLine()
{
    std::vector<Note> v;
    const int pat[] { 69, 72, 76, 81, 79, 76, 72, 76,   65, 69, 72, 77, 76, 72, 69, 72 };
    for (int i = 0; i < 16; ++i) v.push_back ({ i * 0.5, pat[i], 0.4, i % 4 == 0 ? 0.95f : 0.75f });
    v.push_back ({ 8, 57, 2.5, 0.9f }); v.push_back ({ 8, 64, 2.5, 0.85f }); v.push_back ({ 8, 69, 2.5, 0.85f });
    return v;
}
}

int runVideo (int argc, char** argv)
{
    const juce::File dir = juce::File::getCurrentWorkingDirectory().getChildFile (argc > 2 ? argv[2] : "video");
    dir.createDirectory();
    Director d (dir);
    auto& gp = d.ed->generatorPage();

    // ---- intro card over the generator page, with a pad underneath
    {
        gx::GenRequest r; r.category = "Pad"; r.subcategory = "Warm"; r.seed = 4242; r.space = 0.8f;
        gx::Generator::generate (r).applyTo (d.proc);
    }
    d.ed->showPage (0);
    d.play ({ { 0, 45, 5.6, 0.8f }, { 0, 57, 5.6, 0.75f }, { 0, 64, 5.6, 0.75f }, { 0, 71, 5.6, 0.7f } });
    d.card (false, 0.0, 2.6, 0.7);

    // ---- generator: bass
    d.caption ("Smart preset generator", "Pick a category, shape the character, press GENERATE");
    d.click (d.button ("Bass", &gp));
    d.drag (d.genSlider (2), 0.7);                  // aggression
    d.generate();
    d.play (bassRiff);
    d.wait (4.2);

    // ---- pad
    d.caption ("Every sound is rendered, checked and level-matched", "Rule-based sound design, 11 categories, 50+ archetypes");
    d.click (d.button ("Pad", &gp));
    d.drag (d.genSlider (3), 0.85, 0.7);            // space
    d.generate();
    d.play (padChords);
    d.wait (4.6);

    // ---- lead
    d.caption ("Brightness \xc2\xb7 Movement \xc2\xb7 Aggression \xc2\xb7 Space \xc2\xb7 Weirdness");
    d.click (d.button ("Lead", &gp));
    d.drag (d.genSlider (0), 0.75, 0.6);            // brightness
    d.generate();
    d.play (leadMelody);
    d.wait (4.4);

    // ---- mutate + history
    d.caption ("Mutate, blend and browse your session history");
    d.click (d.button ("Mutate", &gp), 0.1);
    for (int i = 0; i < 10 * kFps && gp.isBusy(); ++i) d.frame();
    d.wait (0.3);
    d.play (leadShort);
    d.wait (2.3);
    d.click (d.button ("<", &gp));
    d.play (leadShort);
    d.wait (2.4);
    d.silence();

    // ---- presets
    d.caption ("220 factory presets", "Categories, tags, instant search, favourites, ratings");
    d.click (d.button ("PRESETS"), 0.4);
    d.type (d.find<juce::TextEditor> (d.ed.get()), "arp");
    d.wait (0.3);
    d.selectRow (d.find<juce::ListBox> (d.ed.get(), [] (juce::ListBox* l) { return l->getName() == "presets"; }), 0);
    d.play (heldChords (6));
    d.wait (2.8);

    // ---- editor tour while the arp keeps playing
    d.caption ("Full editor", "4 oscillators: wavetable, analog, FM, noise/sampler and the new Spectral Mutation");
    d.click (d.button ("EDITOR"), 0.2);
    d.click (d.button ("OSC"), 2.2);
    d.caption ("2 filters", "Ladder, SVF, comb and formant, in series or parallel");
    d.click (d.button ("FILTERS"), 2.4);
    d.caption ("Modulation", "4 envelopes \xc2\xb7 4 LFOs \xc2\xb7 8 macros \xc2\xb7 drag-and-drop matrix");
    d.click (d.button ("MOD"), 2.4);
    d.caption ("Rhythm", "Arpeggiator \xc2\xb7 trance gate \xc2\xb7 step sequencer \xc2\xb7 chords & strum");
    d.click (d.button ("ARP / GATE / SEQ"), 2.6);
    d.caption ("12 re-orderable effects", "Including two originals: Shimmer and Granular Smear");
    d.click (d.button ("FX"), 2.4);

    // ---- export + settings
    d.caption ("Export any sound to SFZ", "Multisamples with velocity layers and click-free loops");
    d.click (d.button ("EXPORT"), 2.4);
    d.caption ("Resizable 50\xe2\x80\x93" "200%", "English / Espa\xc3\xb1ol interface");
    d.click (d.button ("SETTINGS"), 2.0);
    d.silence();

    // ---- one more generation, then the end card
    d.caption ("Your next sound is one click away");
    d.click (d.button ("GENERATOR"), 0.2);
    d.click (d.button ("Pluck", &gp));
    d.generate();
    d.clearCaption();
    d.play (pluckLine());
    d.wait (4.6);
    d.card (true, 0.8, 3.6, 0.0);

    d.finishAudio();
    std::printf ("video frames written to %s\n", dir.getChildFile ("frames").getFullPathName().toRawUTF8());
    return 0;
}
