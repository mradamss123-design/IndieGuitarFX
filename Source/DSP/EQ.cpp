#include "EQ.h"

void GuitarEQ::prepare(const juce::dsp::ProcessSpec& spec, juce::AudioProcessorValueTreeState& apvts)
{
    sampleRate = spec.sampleRate;
    updateCoeffs(apvts);
    lowL.reset(); lowR.reset();
    lowMidL.reset(); lowMidR.reset();
    highMidL.reset(); highMidR.reset();
    highL.reset(); highR.reset();
}

void GuitarEQ::updateCoeffs(juce::AudioProcessorValueTreeState& apvts)
{
    float low     = apvts.getRawParameterValue("eq_low")->load();
    float lowMid  = apvts.getRawParameterValue("eq_lowmid")->load();
    float highMid = apvts.getRawParameterValue("eq_highmid")->load();
    float high    = apvts.getRawParameterValue("eq_high")->load();
    float lowF     = apvts.getRawParameterValue("eq_low_freq")->load();
    float lowMidF  = apvts.getRawParameterValue("eq_lowmid_freq")->load();
    float highMidF = apvts.getRawParameterValue("eq_highmid_freq")->load();
    float highF    = apvts.getRawParameterValue("eq_high_freq")->load();

    if (low != lastLow || lowF != lastLowF)
    {
        auto c = Coeffs::makeLowShelf(sampleRate, lowF, 0.707f, juce::Decibels::decibelsToGain(low));
        lowL.coefficients = c; lowR.coefficients = c;
        lastLow = low; lastLowF = lowF;
    }
    if (lowMid != lastLowMid || lowMidF != lastLowMidF)
    {
        auto c = Coeffs::makePeakFilter(sampleRate, lowMidF, 1.0f, juce::Decibels::decibelsToGain(lowMid));
        lowMidL.coefficients = c; lowMidR.coefficients = c;
        lastLowMid = lowMid; lastLowMidF = lowMidF;
    }
    if (highMid != lastHighMid || highMidF != lastHighMidF)
    {
        auto c = Coeffs::makePeakFilter(sampleRate, highMidF, 1.0f, juce::Decibels::decibelsToGain(highMid));
        highMidL.coefficients = c; highMidR.coefficients = c;
        lastHighMid = highMid; lastHighMidF = highMidF;
    }
    if (high != lastHigh || highF != lastHighF)
    {
        auto c = Coeffs::makeHighShelf(sampleRate, highF, 0.707f, juce::Decibels::decibelsToGain(high));
        highL.coefficients = c; highR.coefficients = c;
        lastHigh = high; lastHighF = highF;
    }
}

void GuitarEQ::process(juce::dsp::ProcessContextReplacing<float>& ctx, juce::AudioProcessorValueTreeState& apvts)
{
    updateCoeffs(apvts);

    auto& block = ctx.getOutputBlock();
    int numChannels = (int)block.getNumChannels();
    int numSamples  = (int)block.getNumSamples();

    for (int ch = 0; ch < numChannels && ch < 2; ++ch)
    {
        auto* data = block.getChannelPointer((size_t)ch);
        auto& fLow     = (ch == 0) ? lowL     : lowR;
        auto& fLowMid  = (ch == 0) ? lowMidL  : lowMidR;
        auto& fHighMid = (ch == 0) ? highMidL : highMidR;
        auto& fHigh    = (ch == 0) ? highL    : highR;

        for (int i = 0; i < numSamples; ++i)
        {
            float s = data[i];
            s = fLow.processSample(s);
            s = fLowMid.processSample(s);
            s = fHighMid.processSample(s);
            s = fHigh.processSample(s);
            data[i] = s;
        }
    }
}
