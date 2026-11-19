#pragma once
#include "../preset/Preset.h"

namespace gx
{
// Browser-like history of generated presets, persisted to JSON.
class GenHistory
{
public:
    GenHistory (juce::File f, int limit) : file (std::move (f)), maxSize (juce::jmax (1, limit)) { load(); }
    void setLimit (int l) { maxSize = juce::jmax (1, l); trim(); }
    void push (const Preset& p)
    {
        if (pos + 1 < (int) items.size()) items.erase (items.begin() + pos + 1, items.end()); // cut forward branch
        items.push_back (p);
        trim();
        pos = (int) items.size() - 1;
        save();
    }
    bool canBack() const { return pos > 0; }
    bool canForward() const { return pos + 1 < (int) items.size(); }
    const Preset* back() { if (! canBack()) return nullptr; --pos; save(); return &items[(size_t) pos]; }
    const Preset* forward() { if (! canForward()) return nullptr; ++pos; save(); return &items[(size_t) pos]; }
    const Preset* current() const { return pos >= 0 && pos < (int) items.size() ? &items[(size_t) pos] : nullptr; }
    const Preset* previous() const { return pos > 0 ? &items[(size_t) pos - 1] : nullptr; }
    int index() const { return pos; }
    int size() const { return (int) items.size(); }

private:
    void trim()
    {
        while ((int) items.size() > maxSize) { items.erase (items.begin()); --pos; }
        pos = juce::jlimit (-1, (int) items.size() - 1, pos);
    }
    void save() const
    {
        juce::Array<juce::var> arr;
        for (auto& p : items) arr.add (p.toVar());
        auto* o = new juce::DynamicObject();
        o->setProperty ("version", 1);
        o->setProperty ("pos", pos);
        o->setProperty ("items", arr);
        file.getParentDirectory().createDirectory();
        file.replaceWithText (juce::JSON::toString (juce::var (o), true));
    }
    void load()
    {
        juce::var v;
        if (juce::JSON::parse (file.loadFileAsString(), v).failed()) return;
        if (auto* a = v.getProperty ("items", {}).getArray())
            for (auto& it : *a) items.push_back (Preset::fromVar (it));
        pos = juce::jlimit (-1, (int) items.size() - 1, (int) v.getProperty ("pos", (int) items.size() - 1));
        trim();
    }
    juce::File file;
    int maxSize;
    std::vector<Preset> items;
    int pos = -1;
};
}
