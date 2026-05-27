#include "Chorus.h"
#include <cmath>

void Chorus::prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState&)
{
    sampleRate = spec.sampleRate;
    bufferSize = (int)(sampleRate * maxDelayMs / 1000.0) + 2;
    delayBufferL.assign((size_t)bufferSize, 0.f);
    delayBufferR.assign((size_t)bufferSize, 0.f);
    writePos = 0;
    lfoPhaseL = 0.f;
    lfoPhaseR = juce::MathConstants<float>::pi * 0.5f; // 90 deg offset for stereo
}

float Chorus::readInterpolated(const std::vector<float>& buf, float readPos) const
{
    int pos0 = (int)readPos;
    float frac = readPos - pos0;
    int pos1 = (pos0 + 1) % bufferSize;
    pos0 = ((pos0 % bufferSize) + bufferSize) % bufferSize;
    return buf[(size_t)pos0] * (1.f - frac) + buf[(size_t)pos1] * frac;
}

void Chorus::process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts)
{
    auto& block = ctx.getOutputBlock();
    int numChannels = (int)block.getNumChannels();
    int numSamples  = (int)block.getNumSamples();

    float rate     = apvts.getRawParameterValue("ch_rate")->load();
    float depth    = apvts.getRawParameterValue("ch_depth")->load();
    float mix      = apvts.getRawParameterValue("ch_mix")->load();
    float feedback = apvts.getRawParameterValue("ch_feedback")->load();

    // Depth controls modulation range: 1..15 ms
    float maxDepthMs = 1.f + depth * 14.f;
    float maxDepthSamples = (float)(sampleRate * maxDepthMs / 1000.0);
    float centerDelay = (float)(sampleRate * 7.0 / 1000.0); // 7 ms center

    float lfoInc = rate / (float)sampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        float lfoL = std::sin(lfoPhaseL * juce::MathConstants<float>::twoPi);
        float lfoR = std::sin(lfoPhaseR * juce::MathConstants<float>::twoPi);

        float delayL = centerDelay + lfoL * maxDepthSamples * 0.5f;
        float delayR = centerDelay + lfoR * maxDepthSamples * 0.5f;

        lfoPhaseL = std::fmod(lfoPhaseL + lfoInc, 1.f);
        lfoPhaseR = std::fmod(lfoPhaseR + lfoInc, 1.f);

        for (int ch = 0; ch < numChannels && ch < 2; ++ch)
        {
            auto* data = block.getChannelPointer((size_t)ch);
            auto& buf  = (ch == 0) ? delayBufferL : delayBufferR;
            float delay = (ch == 0) ? delayL : delayR;

            float readPos = (float)writePos - delay;
            if (readPos < 0.f) readPos += (float)bufferSize;

            float wet = readInterpolated(buf, readPos);
            buf[(size_t)writePos] = data[i] + wet * feedback;
            data[i] = data[i] * (1.f - mix) + wet * mix;
        }

        writePos = (writePos + 1) % bufferSize;
    }
}
