#include "SpectrumAnalyzer.h"
#include <cmath>

SpectrumAnalyzer::SpectrumAnalyzer()
{
    smoothed.fill(0.f);
    fftData.fill(0.f);
    // Timer started in visibilityChanged, not constructor
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::visibilityChanged()
{
    if (isVisible())
        startTimerHz(30);
    else
        stopTimer();
}

void SpectrumAnalyzer::setFFTData(const std::array<float, 512>& data)
{
    juce::CriticalSection::ScopedLockType lock(dataMutex);
    fftData = data;
    newData = true;
}

void SpectrumAnalyzer::timerCallback()
{
    bool hasNew = false;
    {
        juce::CriticalSection::ScopedLockType lock(dataMutex);
        hasNew = newData;
        newData = false;
    }
    if (hasNew) repaint();
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    // Background
    g.setColour(juce::Colour(0xff0d0d1a));
    g.fillRoundedRectangle(bounds, 6.f);

    // Grid lines
    g.setColour(juce::Colour(0xff1e1e3a));
    for (int i = 1; i < 4; ++i)
    {
        float y = h * i / 4.f;
        g.drawHorizontalLine((int)y, 0.f, w);
    }

    // Frequency labels
    g.setColour(juce::Colour(0xff444466));
    g.setFont(9.f);
    const float freqLabels[] = { 100.f, 500.f, 1000.f, 5000.f, 10000.f };
    const char* labels[]     = { "100", "500", "1k", "5k", "10k" };
    for (int i = 0; i < 5; ++i)
    {
        float normX = std::log(freqLabels[i] / 20.f) / std::log(20000.f / 20.f);
        float x = normX * w;
        g.drawSingleLineText(labels[i], (int)x, (int)h - 2);
    }

    std::array<float, 512> data;
    {
        juce::CriticalSection::ScopedLockType lock(dataMutex);
        data = fftData;
    }

    // Smooth + normalize
    float maxVal = 1e-6f;
    for (int i = 1; i < 256; ++i)
        if (data[(size_t)i] > maxVal) maxVal = data[(size_t)i];

    const float decay = 0.85f;
    for (int i = 0; i < 256; ++i)
    {
        float norm = data[(size_t)i] / maxVal;
        smoothed[(size_t)i] = smoothed[(size_t)i] * decay + norm * (1.f - decay);
    }

    // Draw filled spectrum path (log frequency scale)
    juce::Path specPath;
    bool started = false;
    float sampleRate = 44100.f;
    int numBins = 256;

    for (int i = 1; i < numBins; ++i)
    {
        float freq = i * sampleRate / (numBins * 2.f);
        if (freq < 20.f || freq > 20000.f) continue;

        float normX = std::log(freq / 20.f) / std::log(20000.f / 20.f);
        float x = normX * w;
        float val = juce::jlimit(0.f, 1.f, smoothed[(size_t)i]);
        float y = h - val * (h - 8.f);

        if (!started) { specPath.startNewSubPath(x, h); specPath.lineTo(x, y); started = true; }
        else          { specPath.lineTo(x, y); }
    }
    specPath.lineTo(w, h);
    specPath.closeSubPath();

    // Gradient fill
    juce::ColourGradient specGrad(
        juce::Colour(0xffe94560).withAlpha(0.7f), 0.f, 0.f,
        juce::Colour(0xff533483).withAlpha(0.3f), 0.f, h, false);
    g.setGradientFill(specGrad);
    g.fillPath(specPath);

    // Stroke
    g.setColour(juce::Colour(0xffe94560));
    g.strokePath(specPath, juce::PathStrokeType(1.5f));

    // Border
    g.setColour(juce::Colour(0xff2a2a4a));
    g.drawRoundedRectangle(bounds, 6.f, 1.f);
}
