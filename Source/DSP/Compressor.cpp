#include "Compressor.h"

void GuitarCompressor::prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts)
{
    sampleRate = spec.sampleRate;
    comp.prepare(spec);
    makeupGain.prepare(spec);

    float threshold = apvts.getRawParameterValue("comp_threshold")->load();
    float ratio     = apvts.getRawParameterValue("comp_ratio")->load();
    float attack    = apvts.getRawParameterValue("comp_attack")->load();
    float release   = apvts.getRawParameterValue("comp_release")->load();
    float gain      = apvts.getRawParameterValue("comp_gain")->load();

    comp.setThreshold(threshold);
    comp.setRatio(ratio);
    comp.setAttack(attack);
    comp.setRelease(release);
    makeupGain.setGainDecibels(gain);
}

void GuitarCompressor::process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts)
{
    float threshold = apvts.getRawParameterValue("comp_threshold")->load();
    float ratio     = apvts.getRawParameterValue("comp_ratio")->load();
    float attack    = apvts.getRawParameterValue("comp_attack")->load();
    float release   = apvts.getRawParameterValue("comp_release")->load();
    float gain      = apvts.getRawParameterValue("comp_gain")->load();

    comp.setThreshold(threshold);
    comp.setRatio(ratio);
    comp.setAttack(attack);
    comp.setRelease(release);
    makeupGain.setGainDecibels(gain);

    comp.process(ctx);
    makeupGain.process(ctx);
}
