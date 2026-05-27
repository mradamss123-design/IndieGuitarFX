#pragma once
#include <JuceHeader.h>

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void setFFTData(const std::array<float, 512>& data);
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void visibilityChanged() override;

private:
    std::array<float, 512> fftData{};
    std::array<float, 512> smoothed{};
    juce::CriticalSection dataMutex;  // safer than std::mutex in JUCE
    bool newData = false;
};
