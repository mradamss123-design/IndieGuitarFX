#include "Delay.h"
#include <cmath>

void GuitarDelay::prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState&)
{
    sampleRate = spec.sampleRate;
    bufferSize = (int)(sampleRate * maxDelayMs / 1000.0) + 2;
    bufL.assign((size_t)bufferSize, 0.f);
    bufR.assign((size_t)bufferSize, 0.f);
    writePos = 0;

    auto lpCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 4000.f);
    lowPassL.coefficients = lpCoeff;
    lowPassR.coefficients = lpCoeff;
    lowPassL.reset();
    lowPassR.reset();
}

float GuitarDelay::readInterpolated(const std::vector<float>& buf, float delaySamples) const
{
    float readPos = (float)writePos - delaySamples;
    while (readPos < 0.f) readPos += (float)bufferSize;
    int pos0 = (int)readPos % bufferSize;
    int pos1 = (pos0 + 1) % bufferSize;
    float frac = readPos - std::floor(readPos);
    return buf[(size_t)pos0] * (1.f - frac) + buf[(size_t)pos1] * frac;
}

void GuitarDelay::process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts)
{
    auto& block = ctx.getOutputBlock();
    int numChannels = (int)block.getNumChannels();
    int numSamples  = (int)block.getNumSamples();

    float delayMs   = apvts.getRawParameterValue("del_time")->load();
    float feedback  = apvts.getRawParameterValue("del_feedback")->load();
    float mix       = apvts.getRawParameterValue("del_mix")->load();

    float delaySamples = (float)(sampleRate * delayMs / 1000.0);
    delaySamples = juce::jlimit(1.f, (float)(bufferSize - 2), delaySamples);

    // Ping-pong: right channel gets offset delay for stereo width
    float delaySamplesR = delaySamples * 1.02f;
    delaySamplesR = juce::jlimit(1.f, (float)(bufferSize - 2), delaySamplesR);

    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < numChannels && ch < 2; ++ch)
        {
            auto* data = block.getChannelPointer((size_t)ch);
            auto& buf  = (ch == 0) ? bufL : bufR;
            auto& lp   = (ch == 0) ? lowPassL : lowPassR;
            float ds   = (ch == 0) ? delaySamples : delaySamplesR;

            float wet = readInterpolated(buf, ds);
            wet = lp.processSample(wet); // darken repeats (tape echo feel)

            buf[(size_t)writePos] = data[i] + wet * feedback;
            data[i] = data[i] * (1.f - mix) + wet * mix;
        }
        writePos = (writePos + 1) % bufferSize;
    }
}
