#pragma once
#include <JuceHeader.h>

class Chorus
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts);
    void process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts);

private:
    double sampleRate = 44100.0;
    static constexpr int maxDelayMs = 30;

    std::vector<float> delayBufferL, delayBufferR;
    int writePos = 0;
    int bufferSize = 0;

    float lfoPhaseL = 0.f;
    float lfoPhaseR = 0.f;   // offset for stereo width

    float readInterpolated(const std::vector<float>& buf, float readPos) const;
};
