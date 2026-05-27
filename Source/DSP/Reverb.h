#pragma once
#include <JuceHeader.h>

class GuitarReverb
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts);
    void process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts);

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters params;
    int lastType = -1;
    float lastSize = -1.f, lastDamp = -1.f, lastWidth = -1.f;
};
