#include "Reverb.h"

void GuitarReverb::prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts)
{
    reverb.prepare(spec);

    float size  = apvts.getRawParameterValue("rev_size")->load();
    float damp  = apvts.getRawParameterValue("rev_damping")->load();
    float width = apvts.getRawParameterValue("rev_width")->load();
    float mix   = apvts.getRawParameterValue("rev_mix")->load();

    params.roomSize   = size;
    params.damping    = damp;
    params.width      = width;
    params.wetLevel   = mix;
    params.dryLevel   = 1.f - mix * 0.5f;
    params.freezeMode = 0.f;

    reverb.setParameters(params);
}

void GuitarReverb::process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts)
{
    float size  = apvts.getRawParameterValue("rev_size")->load();
    float damp  = apvts.getRawParameterValue("rev_damping")->load();
    float width = apvts.getRawParameterValue("rev_width")->load();
    float mix   = apvts.getRawParameterValue("rev_mix")->load();
    int   type  = (int)apvts.getRawParameterValue("rev_type")->load();

    // Type presets
    float roomSizeAdj = size;
    float dampAdj     = damp;
    float widthAdj    = width;

    switch (type)
    {
        case 0: // Room
            roomSizeAdj *= 0.6f;
            dampAdj = juce::jlimit(0.f, 1.f, damp * 1.2f);
            break;
        case 1: // Hall
            roomSizeAdj = juce::jlimit(0.f, 1.f, size * 1.4f + 0.2f);
            dampAdj = damp * 0.7f;
            break;
        case 2: // Plate
            roomSizeAdj = juce::jlimit(0.f, 1.f, size * 1.2f + 0.1f);
            dampAdj = juce::jlimit(0.f, 1.f, damp * 0.5f + 0.1f);
            widthAdj = 1.f;
            break;
        case 3: // Spring
            roomSizeAdj = juce::jlimit(0.f, 1.f, size * 0.5f + 0.1f);
            dampAdj = juce::jlimit(0.f, 1.f, damp * 1.5f);
            break;
        default: break;
    }

    if (roomSizeAdj != lastSize || dampAdj != lastDamp || widthAdj != lastWidth || type != lastType)
    {
        params.roomSize   = roomSizeAdj;
        params.damping    = dampAdj;
        params.width      = widthAdj;
        params.wetLevel   = mix;
        params.dryLevel   = 1.f - mix * 0.5f;
        reverb.setParameters(params);
        lastSize = roomSizeAdj;
        lastDamp = dampAdj;
        lastWidth = widthAdj;
        lastType = type;
    }

    reverb.process(ctx);
}
