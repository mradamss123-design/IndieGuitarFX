#pragma once
#include <JuceHeader.h>

class GuitarDelay
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts);
    void process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts);

private:
    double sampleRate = 44100.0;
    static constexpr int maxDelayMs = 1001;

    std::vector<float> bufL, bufR;
    int writePos = 0;
    int bufferSize = 0;

    juce::dsp::IIR::Filter<float> lowPassL, lowPassR; // darken repeats

    float readInterpolated(const std::vector<float>& buf, float delaySamples) const;
};
