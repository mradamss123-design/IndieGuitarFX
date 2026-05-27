#include "ToneAnalyzer.h"
#include <cmath>
#include <thread>
#include <numeric>

float ToneAnalyzer::computeSpectralCentroid(const std::vector<float>& magnitudes, float binHz) const
{
    float weightedSum = 0.f, totalSum = 0.f;
    for (int i = 0; i < (int)magnitudes.size(); ++i)
    {
        float freq = i * binHz;
        float mag  = magnitudes[(size_t)i];
        weightedSum += freq * mag;
        totalSum    += mag;
    }
    return totalSum > 0.f ? weightedSum / totalSum : 1000.f;
}

float ToneAnalyzer::computeBandEnergy(const std::vector<float>& magnitudes, float binHz, float lowHz, float highHz) const
{
    float energy = 0.f;
    for (int i = 0; i < (int)magnitudes.size(); ++i)
    {
        float freq = i * binHz;
        if (freq >= lowHz && freq <= highHz)
            energy += magnitudes[(size_t)i] * magnitudes[(size_t)i];
    }
    return energy;
}

float ToneAnalyzer::computeTHD(const std::vector<float>& magnitudes, float fundamentalHz, float binHz) const
{
    int fundBin = juce::roundToInt(fundamentalHz / binHz);
    if (fundBin >= (int)magnitudes.size() || fundBin == 0) return 0.f;

    float fundPower = magnitudes[(size_t)fundBin] * magnitudes[(size_t)fundBin];
    float harmonicPower = 0.f;
    for (int h = 2; h <= 5; ++h)
    {
        int hBin = fundBin * h;
        if (hBin < (int)magnitudes.size())
            harmonicPower += magnitudes[(size_t)hBin] * magnitudes[(size_t)hBin];
    }
    return fundPower > 0.f ? std::sqrt(harmonicPower / fundPower) : 0.f;
}

ToneProfile ToneAnalyzer::analyzeFile(const juce::File& audioFile)
{
    ToneProfile profile;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(audioFile));

    if (!reader) return profile;

    int fftOrder = 12; // 4096 points
    int fftSize  = 1 << fftOrder;

    juce::dsp::FFT fft(fftOrder);
    juce::dsp::WindowingFunction<float> window((size_t)fftSize, juce::dsp::WindowingFunction<float>::hann);

    std::vector<float> interleaved((size_t)fftSize * 2, 0.f);
    std::vector<float> fftBuf((size_t)fftSize * 2, 0.f);
    std::vector<float> avgMag((size_t)(fftSize / 2), 0.f);

    int numFrames = 0;
    int totalSamples = (int)reader->lengthInSamples;
    int hop = fftSize / 2;
    float sampleRate = (float)reader->sampleRate;
    float binHz = sampleRate / (float)fftSize;

    // Process in overlapping frames
    for (int offset = 0; offset + fftSize <= totalSamples; offset += hop)
    {
        juce::AudioBuffer<float> buf(1, fftSize);
        reader->read(&buf, 0, fftSize, offset, true, true);

        for (int i = 0; i < fftSize; ++i)
            fftBuf[(size_t)i] = buf.getSample(0, i);

        window.multiplyWithWindowingTable(fftBuf.data(), (size_t)fftSize);
        std::fill(fftBuf.begin() + fftSize, fftBuf.end(), 0.f);

        fft.performFrequencyOnlyForwardTransform(fftBuf.data());

        for (int i = 0; i < fftSize / 2; ++i)
            avgMag[(size_t)i] += fftBuf[(size_t)i];

        ++numFrames;
        progress = (float)(offset) / (float)totalSamples;
    }

    if (numFrames == 0) return profile;

    for (auto& m : avgMag) m /= (float)numFrames;

    // Total energy for normalization
    float totalE = computeBandEnergy(avgMag, binHz, 20.f, 20000.f);
    if (totalE < 1e-10f) return profile;

    float bassE    = computeBandEnergy(avgMag, binHz, 80.f, 300.f)   / totalE;
    float lowMidE  = computeBandEnergy(avgMag, binHz, 300.f, 800.f)  / totalE;
    float midE     = computeBandEnergy(avgMag, binHz, 800.f, 2500.f) / totalE;
    float highMidE = computeBandEnergy(avgMag, binHz, 2500.f, 6000.f)/ totalE;
    float trebleE  = computeBandEnergy(avgMag, binHz, 6000.f, 20000.f)/ totalE;

    float centroid = computeSpectralCentroid(avgMag, binHz);
    float thd      = computeTHD(avgMag, 110.f, binHz); // around A2 guitar note

    // Stereo width estimate
    float stereoWidth = 0.5f;
    if (reader->numChannels >= 2)
    {
        juce::AudioBuffer<float> stereoBlock(2, juce::jmin((int)reader->lengthInSamples, 44100));
        reader->read(&stereoBlock, 0, stereoBlock.getNumSamples(), 0, true, true);
        float mid = 0.f, side = 0.f;
        for (int i = 0; i < stereoBlock.getNumSamples(); ++i)
        {
            float l = stereoBlock.getSample(0, i);
            float r = stereoBlock.getSample(1, i);
            mid  += std::abs(l + r);
            side += std::abs(l - r);
        }
        if (mid > 0.f) stereoWidth = juce::jlimit(0.f, 1.f, side / mid);
    }

    // Map to profile
    profile.bassAmount   = juce::jlimit(0.f, 1.f, bassE * 5.f);
    profile.midAmount    = juce::jlimit(0.f, 1.f, (lowMidE + midE) * 4.f);
    profile.trebleAmount = juce::jlimit(0.f, 1.f, (highMidE + trebleE) * 5.f);
    profile.brightness   = juce::jlimit(0.f, 1.f, centroid / 8000.f);
    profile.harmonics    = juce::jlimit(0.f, 1.f, thd * 2.f);
    profile.reverbAmount = juce::jlimit(0.f, 1.f, trebleE * 3.f * 0.5f);
    profile.delayAmount  = 0.f; // can't reliably detect delay from file
    profile.stereoWidth  = stereoWidth;

    // Description
    if (profile.harmonics > 0.6f)
        profile.description = "Heavy distorted — like Pixies or Weezer";
    else if (profile.harmonics > 0.3f)
        profile.description = "Light overdrive — indie rock warmth";
    else if (profile.brightness > 0.6f)
        profile.description = "Clean bright — like The Strokes jangle";
    else if (profile.bassAmount > 0.5f)
        profile.description = "Warm and bass-heavy — shoegaze-leaning";
    else
        profile.description = "Balanced indie pop tone";

    return profile;
}

void ToneAnalyzer::applyProfile(const ToneProfile& profile, juce::AudioProcessorValueTreeState& apvts)
{
    // EQ
    if (auto* p = apvts.getParameter("eq_low"))
        p->setValueNotifyingHost(p->convertTo0to1(
            juce::jmap(profile.bassAmount, 0.f, 1.f, -6.f, 6.f)));

    if (auto* p = apvts.getParameter("eq_lowmid"))
        p->setValueNotifyingHost(p->convertTo0to1(
            juce::jmap(profile.midAmount, 0.f, 1.f, -4.f, 4.f)));

    if (auto* p = apvts.getParameter("eq_highmid"))
        p->setValueNotifyingHost(p->convertTo0to1(
            juce::jmap(profile.brightness, 0.f, 1.f, -3.f, 5.f)));

    if (auto* p = apvts.getParameter("eq_high"))
        p->setValueNotifyingHost(p->convertTo0to1(
            juce::jmap(profile.trebleAmount, 0.f, 1.f, -4.f, 6.f)));

    // Overdrive
    if (auto* p = apvts.getParameter("od_drive"))
        p->setValueNotifyingHost(p->convertTo0to1(profile.harmonics));

    if (auto* p = apvts.getParameter("od_on"))
        p->setValueNotifyingHost(profile.harmonics > 0.1f ? 1.f : 0.f);

    // Reverb
    if (auto* p = apvts.getParameter("rev_mix"))
        p->setValueNotifyingHost(p->convertTo0to1(profile.reverbAmount * 0.5f));

    if (auto* p = apvts.getParameter("rev_width"))
        p->setValueNotifyingHost(p->convertTo0to1(profile.stereoWidth));

    // Chorus — if very wide stereo image, suggest some chorus
    if (auto* p = apvts.getParameter("ch_on"))
        p->setValueNotifyingHost(profile.stereoWidth > 0.6f ? 1.f : 0.f);

    if (auto* p = apvts.getParameter("ch_depth"))
        p->setValueNotifyingHost(p->convertTo0to1(profile.stereoWidth * 0.7f));
}

void ToneAnalyzer::startAnalysis(const juce::File& file, juce::AudioProcessorValueTreeState&)
{
    if (isAnalyzing) return;
    isAnalyzing = true;
    analysisComplete = false;
    progress = 0.f;

    if (analysisThread && analysisThread->joinable())
        analysisThread->join();

    // NOTE: apvts is NOT used here — applyProfile must be called from the
    // message thread (GUI timer) to avoid crashing FL Studio.
    analysisThread = std::make_unique<std::thread>([this, file]()
    {
        lastProfile = analyzeFile(file);
        // Signal completion — UI timer will call applyProfile on message thread
        isAnalyzing = false;
        analysisComplete = true;
        progress = 1.f;
    });
    analysisThread->detach();
}
