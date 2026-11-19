#include "GeneratorPage.h"

namespace gx::ui
{
GeneratorPage::GeneratorPage (Context& c) : Page (c)
{
    for (auto* comp : std::initializer_list<juce::Component*> { &bus, &scope, &spectrum, &leds, &vu }) addAndMakeVisible (comp);
}

void GeneratorPage::paint (juce::Graphics& g) { g.fillAll (col::bg); }

void GeneratorPage::resized()
{
    auto r = getLocalBounds().reduced (8);
    bus.setBounds (getLocalBounds());
    auto top = r.removeFromTop (260);
    vu.setBounds (top.removeFromRight (60).reduced (4));
    scope.setBounds (top.removeFromLeft (top.getWidth() * 6 / 10).reduced (4));
    spectrum.setBounds (top.reduced (4));
    leds.setBounds (r.removeFromTop (160).reduced (4));
}

std::unique_ptr<juce::Component> makeSfzPanel (Context&) { return nullptr; }
}
