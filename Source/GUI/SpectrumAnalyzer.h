#pragma once
#include <JuceHeader.h>

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer();
    void setFFTData(const std::array<float, 512>& data);
    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    std::array<float, 512> fftData{};
    std::array<float, 512> smoothed{};
    std::mutex dataMutex;
    bool newData = false;
};
