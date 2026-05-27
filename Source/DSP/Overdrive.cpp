#include "Overdrive.h"
#include <cmath>

void Overdrive::prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState&)
{
    sampleRate = spec.sampleRate;

    auto hipassCoeff = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 720.f);
    lowCutL.coefficients = hipassCoeff;
    lowCutR.coefficients = hipassCoeff;

    auto toneCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 3500.f);
    toneFilterL.coefficients = toneCoeff;
    toneFilterR.coefficients = toneCoeff;

    auto outCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.f);
    outputFilterL.coefficients = outCoeff;
    outputFilterR.coefficients = outCoeff;

    lowCutL.reset(); lowCutR.reset();
    toneFilterL.reset(); toneFilterR.reset();
    outputFilterL.reset(); outputFilterR.reset();
}

float Overdrive::softClip(float x, float drive) noexcept
{
    float gain = 1.f + drive * 40.f;
    float s = x * gain;
    // Asymmetric soft clip (Tube Screamer style)
    if (s > 1.f)        return 2.f / 3.f;
    else if (s < -1.f)  return -2.f / 3.f;
    return s - (s * s * s) / 3.f;
}

float Overdrive::hardClip(float x, float drive) noexcept
{
    float gain = 1.f + drive * 30.f;
    return juce::jlimit(-0.7f, 0.7f, x * gain);
}

float Overdrive::fuzzClip(float x, float drive) noexcept
{
    float gain = 1.f + drive * 80.f;
    float s = x * gain;
    return std::tanh(s) * 0.8f;
}

void Overdrive::updateFilters(float tone, int type)
{
    // Tone knob: blend between low-pass and high-pass character
    float toneFreq = 800.f + tone * 7200.f; // 800 Hz to 8 kHz
    auto toneCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneFreq);
    toneFilterL.coefficients = toneCoeff;
    toneFilterR.coefficients = toneCoeff;

    // Type-specific input filter
    float hpFreq = (type == 0) ? 720.f : (type == 1) ? 80.f : 200.f;
    auto hipassCoeff = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpFreq);
    lowCutL.coefficients = hipassCoeff;
    lowCutR.coefficients = hipassCoeff;
}

void Overdrive::process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts)
{
    auto& block = ctx.getOutputBlock();
    int numChannels = (int)block.getNumChannels();
    int numSamples  = (int)block.getNumSamples();

    float drive  = apvts.getRawParameterValue("od_drive")->load();
    float tone   = apvts.getRawParameterValue("od_tone")->load();
    float level  = apvts.getRawParameterValue("od_level")->load();
    int   type   = (int)apvts.getRawParameterValue("od_type")->load();

    updateFilters(tone, type);

    for (int ch = 0; ch < numChannels && ch < 2; ++ch)
    {
        auto* data = block.getChannelPointer((size_t)ch);
        auto& hipass  = (ch == 0) ? lowCutL      : lowCutR;
        auto& toneFlt = (ch == 0) ? toneFilterL   : toneFilterR;
        auto& outFlt  = (ch == 0) ? outputFilterL : outputFilterR;

        for (int i = 0; i < numSamples; ++i)
        {
            float s = hipass.processSample(data[i]);

            switch (type)
            {
                case 0: s = softClip(s, drive); break;  // Tube Screamer
                case 1: s = hardClip(s, drive); break;  // Blues Driver
                case 2: s = fuzzClip(s, drive); break;  // Fuzz
                default: break;
            }

            s = toneFlt.processSample(s);
            s = outFlt.processSample(s);
            data[i] = s * level * 0.7f;
        }
    }
}
