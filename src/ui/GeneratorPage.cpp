#include "GeneratorPage.h"
#include "../gen/Messages.h"

namespace gx::ui
{
static double nowSec() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }

// ================================================================ GenerateButton
void GenerateButton::burst()
{
    const auto c = getLocalBounds().toFloat().getCentre();
    for (int i = 0; i < 70; ++i)
    {
        const float a = rng.nextFloat() * kTwoPi, s = 60.0f + rng.nextFloat() * 260.0f;
        particles.push_back ({ c, { std::cos (a) * s, std::sin (a) * s }, 1.0f, rng.nextBool() ? col::accent : (rng.nextBool() ? col::accent2 : col::accent3) });
    }
    glow = 1.0f;
}

void GenerateButton::animate (float dt, float energy, bool busy)
{
    working = busy;
    spin += dt * (busy ? 4.0f : 0.35f + energy * 1.5f);
    pulse += dt * 2.0f;
    glow = juce::jmax (0.0f, glow - dt * 1.5f);
    for (auto& p : particles) { p.p += p.v * dt; p.v *= 0.96f; p.life -= dt * 1.1f; }
    particles.erase (std::remove_if (particles.begin(), particles.end(), [] (auto& p) { return p.life <= 0; }), particles.end());
    repaint();
}

void GenerateButton::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const auto c = b.getCentre();
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.36f;
    // outer rotating ring of ticks
    for (int i = 0; i < 48; ++i)
    {
        const float a = spin + (float) i / 48.0f * kTwoPi;
        const float r0 = r * 1.18f, r1 = r * (i % 4 == 0 ? 1.32f : 1.25f);
        g.setColour ((i % 8 == 0 ? col::accent : col::textDim).withAlpha (working ? 0.9f : 0.45f));
        g.drawLine (c.x + std::cos (a) * r0, c.y + std::sin (a) * r0, c.x + std::cos (a) * r1, c.y + std::sin (a) * r1, i % 4 == 0 ? 2.0f : 1.0f);
    }
    // glow
    const float pul = 0.5f + 0.5f * std::sin (pulse);
    g.setColour (col::accent.withAlpha (0.10f + 0.12f * pul + 0.4f * glow + (working ? 0.2f : 0.0f)));
    g.fillEllipse (juce::Rectangle<float> (r * 2.5f, r * 2.5f).withCentre (c));
    // body
    juce::ColourGradient body (col::accent.brighter (pressed ? 0.0f : 0.35f), c.x, c.y - r, col::accent.darker (0.6f), c.x, c.y + r, false);
    g.setGradientFill (body);
    g.fillEllipse (juce::Rectangle<float> (r * 2, r * 2).withCentre (c));
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawEllipse (juce::Rectangle<float> (r * 2 - 3, r * 2 - 3).withCentre (c), 1.5f);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.drawEllipse (juce::Rectangle<float> (r * 2, r * 2).withCentre (c), 2.0f);
    // inner arc spinner while working
    if (working)
    {
        juce::Path arc;
        arc.addCentredArc (c.x, c.y, r * 0.86f, r * 0.86f, 0, spin * 1.7f, spin * 1.7f + 2.2f, true);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.setFont (font (r * 0.34f, true));
    g.drawText (working ? L ("CREANDO…", "CREATING…") : L ("GENERAR", "GENERATE"), b, juce::Justification::centred);
    for (auto& p : particles)
    {
        g.setColour (p.c.withAlpha (juce::jlimit (0.0f, 1.0f, p.life)));
        g.fillEllipse (juce::Rectangle<float> (4, 4).withCentre (p.p));
    }
}

// ================================================================ AiConsole
void AiConsole::start (const juce::StringArray& l, double duration)
{
    lines = l;
    shown.clear();
    lineIdx = 0; typed = 0;
    t0 = nowSec();
    perLine = duration / juce::jmax (1, lines.size());
    running = true;
}

void AiConsole::animate (double t)
{
    now = t;
    if (running)
    {
        const double el = t - t0;
        lineIdx = juce::jmin (lines.size() - 1, (int) (el / perLine));
        const double inLine = (el - lineIdx * perLine) / (perLine * 0.7);
        typed = (int) (juce::jlimit (0.0, 1.0, inLine) * lines[lineIdx].length());
        if (el >= perLine * lines.size()) { running = false; lineIdx = lines.size() - 1; typed = lines[lineIdx].length(); }
    }
    repaint();
}

void AiConsole::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    drawScreen (g, r);
    auto in = r.reduced (12, 8);
    g.setFont (monoFont (13.5f));
    const int maxLines = juce::jmax (1, (int) (in.getHeight() / 19.0f));
    const int first = juce::jmax (0, lineIdx - maxLines + 1);
    for (int i = first; i <= lineIdx && i < lines.size(); ++i)
    {
        const bool cur = i == lineIdx;
        const auto text = cur ? lines[i].substring (0, typed) : lines[i];
        g.setColour (cur ? col::phosphor : col::phosphor.withAlpha (0.45f));
        auto row = in.removeFromTop (19);
        g.drawText ("> " + text, row, juce::Justification::centredLeft);
        if (cur && std::fmod (now, 0.8) < 0.45)
        {
            juce::GlyphArrangement ga; ga.addLineOfText (monoFont (13.5f), "> " + text, 0, 0); const float w = ga.getBoundingBox (0, -1, true).getWidth();
            g.fillRect (row.getX() + w + 2, row.getY() + 3, 8.0f, row.getHeight() - 6);
        }
    }
}

// ================================================================ ResolveGrid
void ResolveGrid::setTarget (const Preset& p)
{
    auto v = [&] (const char* id) { return p.params[(size_t) paramIndex (id)]; };
    auto choice = [&] (const char* id) { const auto& d = paramDefs()[(size_t) paramIndex (id)]; return d.choices[(int) v (id)]; };
    auto secs = [] (float s) { return s < 1 ? juce::String ((int) std::lround (s * 1000)) + " ms" : juce::String (s, 2) + " s"; };
    int fxCount = 0;
    for (auto id : { "dist_on", "chorus_on", "flanger_on", "phaser_on", "delay_on", "reverb_on", "comp_on", "eq_on", "crush_on", "wide_on", "shimmer_on", "granular_on" }) fxCount += v (id) > 0.5f;
    int mods = 0;
    for (int s = 0; s < kNumModSlots; ++s) mods += p.params[(size_t) P::mod (s, P::mSrc)] > 0;
    cells = { { { "OSC A", v ("osc1_on") > 0.5f ? choice ("osc1_type") : juce::String ("-") },
                { "OSC B", v ("osc2_on") > 0.5f ? choice ("osc2_type") : juce::String ("-") },
                { L ("FILTRO", "FILTER"), v ("filter1_on") > 0.5f ? choice ("filter1_type") : juce::String ("bypass") },
                { "CUTOFF", juce::String (v ("filter1_cutoff") / 1000.0f, 2) + " kHz" },
                { "RESO", juce::String ((int) (v ("filter1_reso") * 100)) + " %" },
                { L ("ATAQUE", "ATTACK"), secs (v ("env1_attack")) },
                { "RELEASE", secs (v ("env1_release")) },
                { "UNISON", juce::String ((int) v ("uni_voices")) + " v" },
                { "MOD", juce::String (mods) + L (" rutas", " routes") },
                { "FX", juce::String (fxCount) + L (" activos", " active") },
                { L ("RITMO", "RHYTHM"), v ("arp_on") > 0.5f ? juce::String ("ARP ") + choice ("arp_mode") : (v ("tg_on") > 0.5f ? juce::String ("GATE") : juce::String ("-")) },
                { L ("NIVEL", "LEVEL"), juce::String (v ("master"), 1) + " dB" } } };
    repaint();
}

void ResolveGrid::animate (float s, float a, float d) { scramble = s; assemble = a; disassemble = d; repaint(); }

void ResolveGrid::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    drawScreen (g, r);
    auto in = r.reduced (8, 6);
    const int cols = 4, rows = 3;
    const float cw = in.getWidth() / cols, ch = in.getHeight() / rows;
    static const char glyphs[] = "0123456789ABCDEF#%&@$*+=<>";
    for (int i = 0; i < 12; ++i)
    {
        auto cell = juce::Rectangle<float> (in.getX() + (i % cols) * cw, in.getY() + (i / cols) * ch, cw, ch).reduced (3);
        // per-cell stagger for the assemble / disassemble choreography
        const float stagger = (float) i / 12.0f;
        const float a = juce::jlimit (0.0f, 1.0f, assemble * 1.6f - stagger * 0.6f);
        const float d = juce::jlimit (0.0f, 1.0f, disassemble * 1.6f - stagger * 0.6f);
        const float offset = (1.0f - a) * 18.0f + d * -18.0f;
        const float alpha = juce::jlimit (0.15f, 1.0f, a * (1.0f - d * 0.85f));
        cell = cell.translated (0, offset);
        g.setColour (col::panelLo.withAlpha (0.8f * alpha));
        g.fillRoundedRectangle (cell, 3);
        g.setColour (col::textDim.withAlpha (alpha));
        g.setFont (monoFont (9.5f));
        g.drawText (cells[(size_t) i].label, cell.reduced (6, 2), juce::Justification::topLeft);
        juce::String val = cells[(size_t) i].value;
        if (scramble > 0.01f)
        {
            juce::String sc;
            for (int k = 0; k < juce::jmax (4, val.length()); ++k) sc << (rng.nextFloat() < scramble ? juce::String::charToString ((juce::juce_wchar) glyphs[rng.nextInt (26)]) : val.substring (k, k + 1));
            val = sc;
        }
        g.setColour ((scramble > 0.01f ? col::accent : col::phosphor).withAlpha (alpha));
        g.setFont (monoFont (13.0f));
        g.drawText (val, cell.reduced (6, 2), juce::Justification::bottomLeft);
    }
}

// ================================================================ GeneratorPage
GeneratorPage::GeneratorPage (Context& c) : Page (c), macros (c, "Macros", "macros")
{
    history = std::make_unique<GenHistory> (Library::defaultRoot().getChildFile ("history.json"), ctx.settings.historyLimit());
    addAndMakeVisible (bus);
    for (auto& cat : categories())
    {
        auto* b = catButtons.add (new juce::TextButton (cat));
        b->setRadioGroupId (101);
        b->setClickingTogglesState (true);
        b->onClick = [this, cat] { category = cat; };
        b->setToggleState (cat == category, juce::dontSendNotification);
        addAndMakeVisible (b);
    }
    sliderNames = { L ("Brillo", "Brightness"), L ("Movimiento", "Movement"), L ("Agresividad", "Aggression"), L ("Espacio", "Space"), L ("Rareza", "Weirdness") };
    const float defaults[] { 0.5f, 0.5f, 0.35f, 0.4f, 0.2f };
    for (int i = 0; i < sliderNames.size(); ++i)
    {
        auto* s = sliders.add (new juce::Slider (juce::Slider::LinearVertical, juce::Slider::NoTextBox));
        s->setRange (0, 1, 0.01);
        s->setValue (defaults[i], juce::dontSendNotification);
        s->setPopupDisplayEnabled (true, true, nullptr);
        s->setTooltip (sliderNames[i]);
        s->getProperties().set ("arc", (juce::int64) (i == 4 ? col::accent3 : i == 2 ? col::danger : col::accent).getARGB());
        addAndMakeVisible (s);
    }
    const juce::String lockNames[] { "OSC", L ("FILTRO", "FILTER"), "ENV", "MOD", "FX", L ("RITMO", "RHYTHM") };
    const char* lockIds[] { "osc", "filter", "env", "mod", "fx", "rhythm" };
    for (int i = 0; i < 6; ++i)
    {
        auto* t = locks.add (new juce::ToggleButton (lockNames[i]));
        t->getProperties().set ("lock", lockIds[i]);
        t->setTooltip (L ("Bloquear esta sección: GENERAR y MUTAR la conservan", "Lock this section: GENERATE and MUTATE keep it"));
        addAndMakeVisible (t);
    }
    addAndMakeVisible (genButton);
    genButton.onClick = [this] { startJob (Job::Generate); };
    addAndMakeVisible (console);
    addAndMakeVisible (grid);
    for (auto* b : { &mutateBtn, &blendBtn, &saveBtn, &exportBtn, &editorBtn, &backBtn, &fwdBtn }) addAndMakeVisible (b);
    mutateBtn.setTooltip (L ("Variación suave del preset actual", "Gentle variation of the current preset"));
    blendBtn.setTooltip (L ("Mezclar con otro preset", "Blend with another preset"));
    backBtn.setTooltip (L ("Anterior en el historial (Alt+Izquierda)", "Previous in history (Alt+Left)"));
    fwdBtn.setTooltip (L ("Siguiente en el historial (Alt+Derecha)", "Next in history (Alt+Right)"));
    mutateBtn.onClick = [this] { startJob (Job::Mutate); };
    blendBtn.onClick = [this] { showBlendMenu(); };
    saveBtn.onClick = [this] { saveCurrent(); };
    exportBtn.onClick = [this] { if (openExport) openExport(); };
    editorBtn.onClick = [this] { if (openEditor) openEditor(); };
    backBtn.onClick = [this] { historyStep (-1); };
    fwdBtn.onClick = [this] { historyStep (1); };
    histLabel.setJustificationType (juce::Justification::centred);
    histLabel.setFont (monoFont (13.0f));
    histLabel.setColour (juce::Label::textColourId, col::phosphor);
    addAndMakeVisible (histLabel);
    for (auto* comp : std::initializer_list<juce::Component*> { &scope, &spectrum, &leds, &vu }) addAndMakeVisible (comp);
    macros.accent = col::accent3;
    std::vector<juce::Component*> mk;
    for (int m = 0; m < kNumMacros; ++m)
    {
        const juce::String names[] { L ("BRILLO", "BRIGHT"), "TIMBRE", L ("ESPACIO", "SPACE"), L ("CARÁCTER", "CHARACTER"), "MACRO 5", "MACRO 6", "MACRO 7", "MACRO 8" };
        auto* k = &macros.add<ParamKnob> (c, P::macro1 + m, names[m]);
        k->slider.getProperties().set ("arc", (juce::int64) col::accent3.getARGB());
        mk.push_back (k);
    }
    macros.layout = [mk] (juce::Rectangle<int> r) { gridKnobs (mk, r, 4); };
    addAndMakeVisible (macros);
    grid.setTarget (Preset::capture (ctx.proc));
    console.showIdle (L ("Listo. Elige una categoría y pulsa GENERAR.", "Ready. Pick a category and press GENERATE."));
    updateHistoryLabel();
    startTimerHz (30);
}

GeneratorPage::~GeneratorPage()
{
    stopTimer();
    if (worker.joinable()) worker.join();
}

GenRequest GeneratorPage::makeRequest() const
{
    GenRequest r;
    r.category = category;
    r.brightness = (float) sliders[0]->getValue();
    r.movement = (float) sliders[1]->getValue();
    r.aggression = (float) sliders[2]->getValue();
    r.space = (float) sliders[3]->getValue();
    r.weirdness = (float) sliders[4]->getValue();
    r.seed = (uint32_t) juce::Random::getSystemRandom().nextInt() ^ (uint32_t) juce::Time::currentTimeMillis();
    for (auto* l : locks) if (l->getToggleState()) r.locks.insert (l->getProperties()["lock"].toString());
    return r;
}

void GeneratorPage::startJob (Job j, const Preset* other)
{
    if (busy.load()) return;
    if (worker.joinable()) worker.join();
    busy = true;
    jobDone = false;
    auto req = makeRequest();
    auto current = Preset::capture (ctx.proc);
    std::unique_ptr<Preset> otherCopy = other ? std::make_unique<Preset> (*other) : nullptr;
    worker = std::thread ([this, j, req, current, oc = std::shared_ptr<Preset> (std::move (otherCopy))]() mutable
    {
        Preset out;
        if (j == Job::Generate)
        {
            req.base = &current;
            out = Generator::generate (req);
        }
        else
        {
            // live variations go through the same validation; retry with new seeds, never apply a failing sound
            bool ok = false;
            for (int a = 0; a < 8 && ! ok; ++a)
            {
                const uint32_t seed = req.seed + (uint32_t) a * 104729u;
                out = j == Job::Mutate ? Generator::mutate (current, 0.35f - 0.03f * (float) a, seed, req.locks)
                                       : Generator::blend (current, oc ? *oc : current, 0.5f, seed);
                if (out.meta.category.isEmpty()) out.meta.category = current.meta.category;
                ok = Generator::validate (out).ok;
                if (j == Job::Mutate) out.meta.tags = current.meta.tags;
            }
            jobFailed = ! ok;
        }
        result = std::move (out);
        jobDone = true;
    });
    jobStart = nowSec();
    const bool msgs = ctx.settings.aiMessages();
    minDuration = msgs ? 1.6 : 0.5;
    const auto cat = j == Job::Generate ? category : ctx.proc.currentMeta.category;
    if (msgs) console.start (aiMessages (cat, uiEnglish(), msgRng, 4, lastMsg), minDuration);
    else console.showIdle (L ("Generando...", "Generating..."));
    genButton.burst();
}

void GeneratorPage::finishJob()
{
    if (worker.joinable()) worker.join();
    if (jobFailed.exchange (false))
    {
        busy = false;
        console.showIdle (L ("La variación no pasó la validación: se conserva el sonido actual.", "The variation failed validation: keeping the current sound."));
        if (ctx.toast) ctx.toast (L ("Variación descartada (no pasó la validación de calidad)", "Variation discarded (failed the quality check)"));
        return;
    }
    result.applyTo (ctx.proc);
    history->setLimit (ctx.settings.historyLimit());
    history->push (result);
    grid.setTarget (result);
    assembleStart = nowSec();
    busy = false;
    updateHistoryLabel();
    if (! ctx.settings.aiMessages()) console.showIdle (result.meta.name + "  [" + result.meta.category + " / " + result.meta.subcategory + "]");
    if (onPresetChanged) onPresetChanged();
    ctx.proc.keyboardState.noteOn (1, Generator::testNote (result.meta.category), 0.8f);
    juce::Timer::callAfterDelay (650, [sp = juce::Component::SafePointer<GeneratorPage> (this), n = Generator::testNote (result.meta.category)]
    { if (sp) sp->ctx.proc.keyboardState.noteOff (1, n, 0.0f); });
}

void GeneratorPage::historyStep (int dir)
{
    if (busy.load()) return;
    const Preset* p = dir < 0 ? history->back() : history->forward();
    if (! p) return;
    p->applyTo (ctx.proc);
    grid.setTarget (*p);
    assembleStart = nowSec();
    updateHistoryLabel();
    console.showIdle (p->meta.name + "  [" + p->meta.category + " / " + p->meta.subcategory + "]");
    if (onPresetChanged) onPresetChanged();
}

void GeneratorPage::updateHistoryLabel()
{
    histLabel.setText (juce::String (history->index() + 1) + " / " + juce::String (history->size()), juce::dontSendNotification);
    backBtn.setEnabled (history->canBack());
    fwdBtn.setEnabled (history->canForward());
}

void GeneratorPage::presetLoaded() { grid.setTarget (Preset::capture (ctx.proc)); assembleStart = nowSec(); }

void GeneratorPage::saveCurrent()
{
    auto p = Preset::capture (ctx.proc);
    auto* w = new juce::AlertWindow (L ("Guardar preset generado", "Save generated preset"), "", juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("name", p.meta.name, L ("Nombre", "Name"));
    w->addComboBox ("cat", categories(), L ("Categoría", "Category"));
    w->getComboBoxComponent ("cat")->setText (p.meta.category, juce::dontSendNotification);
    w->addTextEditor ("sub", p.meta.subcategory, L ("Subcategoría", "Subcategory"));
    w->addTextEditor ("tags", p.meta.tags.joinIntoString (", "), L ("Etiquetas", "Tags"));
    w->addTextEditor ("author", L ("Usuario", "User"), L ("Autor", "Author"));
    w->addTextEditor ("notes", p.meta.notes, L ("Notas", "Notes"));
    w->addComboBox ("fav", { L ("No favorito", "Not favourite"), L ("Favorito", "Favourite") }, L ("Favorito", "Favourite"));
    w->addButton (L ("Guardar", "Save"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton (L ("Cancelar", "Cancel"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    w->enterModalState (true, juce::ModalCallbackFunction::create ([this, w, p] (int r) mutable
    {
        if (r != 1) return;
        p.meta.name = w->getTextEditorContents ("name").trim();
        p.meta.category = w->getComboBoxComponent ("cat")->getText();
        p.meta.subcategory = w->getTextEditorContents ("sub").trim();
        p.meta.tags = juce::StringArray::fromTokens (w->getTextEditorContents ("tags"), ",", "");
        p.meta.tags.trim(); p.meta.tags.removeEmptyStrings();
        p.meta.author = w->getTextEditorContents ("author").trim();
        p.meta.notes = w->getTextEditorContents ("notes");
        p.meta.favorite = w->getComboBoxComponent ("fav")->getSelectedItemIndex() == 1;
        p.meta.uuid = {};
        if (ctx.library.saveUser (p))
        {
            ctx.proc.currentMeta = p.meta;
            if (p.meta.favorite) ctx.library.setFavorite (ctx.library.indexOfUuid (p.meta.uuid), true);
            if (ctx.toast) ctx.toast (L ("Guardado en PRESETS: ", "Saved to PRESETS: ") + p.meta.name);
            if (onPresetChanged) onPresetChanged();
        }
    }), true);
}

void GeneratorPage::showBlendMenu()
{
    juce::PopupMenu m;
    m.addItem (1, L ("Con el anterior del historial", "With the previous one in history"), history->previous() != nullptr);
    m.addItem (2, L ("Con un favorito al azar", "With a random favourite"));
    m.addItem (3, L ("Con un preset de la misma categoría", "With a preset from the same category"));
    m.addItem (4, L ("Con un preset cualquiera de la biblioteca", "With any preset from the library"));
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&blendBtn), [this] (int r)
    {
        if (r == 1) { if (auto* p = history->previous()) startJob (Job::Blend, p); return; }
        if (r < 2) return;
        PresetFilter f;
        if (r == 2) f.favoritesOnly = true;
        if (r == 3) f.category = ctx.proc.currentMeta.category;
        auto idx = ctx.library.query (f);
        if (idx.empty()) { if (ctx.toast) ctx.toast (L ("No hay presets para mezclar con ese criterio", "No presets to blend with for that choice")); return; }
        Preset other;
        if (ctx.library.load (idx[(size_t) juce::Random::getSystemRandom().nextInt ((int) idx.size())], other))
        {
            if (ctx.toast) ctx.toast (L ("Mezclando con: ", "Blending with: ") + other.meta.name);
            startJob (Job::Blend, &other);
        }
    });
}

bool GeneratorPage::handleKey (const juce::KeyPress& k)
{
    if (k.getModifiers().isAltDown() && k.getKeyCode() == juce::KeyPress::leftKey) { historyStep (-1); return true; }
    if (k.getModifiers().isAltDown() && k.getKeyCode() == juce::KeyPress::rightKey) { historyStep (1); return true; }
    if (k.getModifiers().isCommandDown() && k.getKeyCode() == juce::KeyPress::returnKey) { startJob (Job::Generate); return true; }
    return false;
}

void GeneratorPage::timerCallback()
{
    if (busy.load() && jobDone.load() && nowSec() - jobStart >= minDuration && console.finished()) finishJob();
    else if (busy.load() && jobDone.load() && nowSec() - jobStart >= minDuration + 1.0) finishJob(); // safety
}

void GeneratorPage::tick (const Frame& f)
{
    Page::tick (f);
    const double t = nowSec();
    const float dt = (float) juce::jlimit (0.0, 0.1, t - lastAnim);
    lastAnim = t;
    energy = f.energy;
    genButton.animate (dt, energy, busy.load());
    console.animate (t);
    const bool b = busy.load();
    const float dis = b ? (float) juce::jlimit (0.0, 1.0, (t - jobStart) / 0.35) : 0.0f;
    const float as = (float) juce::jlimit (0.0, 1.0, (t - assembleStart) / 0.6);
    grid.animate (b ? 0.9f : juce::jmax (0.0f, 1.0f - as * 1.4f), b ? 0.0f : as, dis);
    scope.label = b ? L ("ENSAMBLANDO…", "ASSEMBLING…") : L ("OSCILOSCOPIO", "OSCILLOSCOPE");
}

void GeneratorPage::paint (juce::Graphics& g)
{
    g.fillAll (col::bg);
    auto r = getLocalBounds().reduced (8);
    auto left = r.removeFromLeft (290);
    drawPanel (g, left.toFloat());
    g.setColour (col::text);
    g.setFont (font (12.0f, true));
    auto l = left.reduced (12, 8);
    g.drawText (L ("CATEGORÍA", "CATEGORY"), l.removeFromTop (16), juce::Justification::centredLeft);
    l.removeFromTop (6 * 30 + 8);
    g.drawText (L ("CARÁCTER", "CHARACTER"), l.removeFromTop (16), juce::Justification::centredLeft);
    auto sl = l.removeFromTop (230);
    g.setFont (font (10.5f, true));
    g.setColour (col::textDim);
    const int sw = sl.getWidth() / sliderNames.size();
    for (int i = 0; i < sliderNames.size(); ++i)
        g.drawFittedText (juce::StringArray { L ("BRILLO", "BRIGHT"), L ("MOVIM.", "MOVEM."), L ("AGRES.", "AGGR."), L ("ESPACIO", "SPACE"), L ("RAREZA", "WEIRD") }[i], sl.getX() + i * sw, sl.getBottom() - 16, sw, 16, juce::Justification::centred, 1, 0.6f);
    g.setColour (col::text);
    g.setFont (font (12.0f, true));
    l.removeFromTop (8);
    g.drawText (L ("BLOQUEAR SECCIONES", "LOCK SECTIONS"), l.removeFromTop (16), juce::Justification::centredLeft);
}

void GeneratorPage::resized()
{
    bus.setBounds (getLocalBounds());
    auto r = getLocalBounds().reduced (8);
    auto left = r.removeFromLeft (290).reduced (12, 8);
    left.removeFromTop (20);
    auto cats = left.removeFromTop (6 * 30);
    for (int i = 0; i < catButtons.size(); ++i)
        catButtons[i]->setBounds (juce::Rectangle<int> (cats.getX() + (i % 2) * cats.getWidth() / 2, cats.getY() + (i / 2) * 30, cats.getWidth() / 2, 30).reduced (2));
    left.removeFromTop (8 + 18);
    auto sl = left.removeFromTop (230).withTrimmedBottom (18);
    const int sw = sl.getWidth() / sliders.size();
    for (int i = 0; i < sliders.size(); ++i) sliders[i]->setBounds (sl.getX() + i * sw, sl.getY(), sw, sl.getHeight());
    left.removeFromTop (8 + 18);
    for (int i = 0; i < locks.size(); ++i)
        locks[i]->setBounds (juce::Rectangle<int> (left.getX() + (i % 3) * left.getWidth() / 3, left.getY() + (i / 3) * 26, left.getWidth() / 3, 26));

    r.removeFromLeft (8);
    auto right = r.removeFromRight (370);
    r.removeFromRight (8);
    // centre
    auto c = r;
    genButton.setBounds (c.removeFromTop (220).withSizeKeepingCentre (240, 220));
    auto hist = c.removeFromTop (30);
    backBtn.setBounds (hist.withSizeKeepingCentre (160, 26).removeFromLeft (34));
    fwdBtn.setBounds (hist.withSizeKeepingCentre (160, 26).removeFromRight (34));
    histLabel.setBounds (hist.withSizeKeepingCentre (90, 26));
    c.removeFromTop (6);
    console.setBounds (c.removeFromTop (130));
    c.removeFromTop (8);
    grid.setBounds (c.removeFromTop (140));
    c.removeFromTop (10);
    auto act = c.removeFromTop (36);
    const int aw = act.getWidth() / 5;
    for (auto* b : { &mutateBtn, &blendBtn, &saveBtn, &exportBtn, &editorBtn }) b->setBounds (act.removeFromLeft (aw).reduced (3, 0));
    // right
    auto top = right.removeFromTop (190);
    vu.setBounds (top.removeFromRight (44).reduced (2, 0));
    scope.setBounds (top.reduced (2, 0));
    right.removeFromTop (6);
    spectrum.setBounds (right.removeFromTop (110).reduced (2, 0));
    right.removeFromTop (6);
    leds.setBounds (right.removeFromTop (150).reduced (2, 0));
    right.removeFromTop (6);
    macros.setBounds (right.reduced (2, 0));
}

}
