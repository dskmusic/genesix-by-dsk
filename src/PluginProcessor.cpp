#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "master", 1 }, "Master", 0.0f, 1.0f, 0.7f));
    return l;
}

GenesixProcessor::GenesixProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "GENESIX", makeLayout())
{
}

void GenesixProcessor::prepareToPlay (double, int) {}

bool GenesixProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void GenesixProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    buffer.clear();
}

juce::AudioProcessorEditor* GenesixProcessor::createEditor() { return new GenesixEditor (*this); }

void GenesixProcessor::getStateInformation (juce::MemoryBlock& mb)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, mb);
}

void GenesixProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}
