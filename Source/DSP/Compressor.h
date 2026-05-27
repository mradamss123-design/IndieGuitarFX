#pragma once
#include <JuceHeader.h>

class GuitarCompressor
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts);
    void process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts);

private:
    juce::dsp::Compressor<float> comp;
    juce::dsp::Gain<float> makeupGain;
    double sampleRate = 44100.0;
};
