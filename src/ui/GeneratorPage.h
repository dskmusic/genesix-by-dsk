#pragma once
#include "Pages.h"

namespace gx::ui
{
class GeneratorPage : public Page
{
public:
    explicit GeneratorPage (Context& c);
    void resized() override;
    void paint (juce::Graphics&) override;
    bool handleKey (const juce::KeyPress&) { return false; }
    void presetLoaded() {}
    std::function<void()> onPresetChanged, openEditor, openExport, openPresets;
private:
    Scope scope;
    Spectrum spectrum;
    LedMatrix leds;
    VuMeter vu;
    CircuitBus bus;
};

std::unique_ptr<juce::Component> makeSfzPanel (Context& c);
}
