#pragma once
#include <JuceHeader.h>

class Overdrive
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts);
    void process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts);

private:
    double sampleRate = 44100.0;

    // Pre/post filters for Tube Screamer tone stack
    juce::dsp::IIR::Filter<float> lowCutL, lowCutR;
    juce::dsp::IIR::Filter<float> toneFilterL, toneFilterR;
    juce::dsp::IIR::Filter<float> outputFilterL, outputFilterR;

    float softClip(float x, float drive) noexcept;
    float hardClip(float x, float drive) noexcept;
    float fuzzClip(float x, float drive) noexcept;

    void updateFilters(float tone, int type);
};
