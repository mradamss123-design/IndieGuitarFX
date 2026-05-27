#pragma once
#include <JuceHeader.h>

class GuitarEQ
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts);
    void process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts);

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    // 4 bands x 2 channels
    Filter lowL, lowR;
    Filter lowMidL, lowMidR;
    Filter highMidL, highMidR;
    Filter highL, highR;

    double sampleRate = 44100.0;
    void updateCoeffs(juce::AudioProcessorValueTreeState& apvts);

    float lastLow = -999.f, lastLowMid = -999.f, lastHighMid = -999.f, lastHigh = -999.f;
    float lastLowF = -1.f, lastLowMidF = -1.f, lastHighMidF = -1.f, lastHighF = -1.f;
};
