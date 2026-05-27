#pragma once
#include <JuceHeader.h>

struct ToneProfile
{
    float bassAmount    = 0.5f;  // 0..1
    float midAmount     = 0.5f;
    float trebleAmount  = 0.5f;
    float brightness    = 0.5f;
    float harmonics     = 0.5f;  // distortion estimate
    float reverbAmount  = 0.5f;
    float delayAmount   = 0.5f;
    float stereoWidth   = 0.5f;
    juce::String description = "Neutral";
};

class ToneAnalyzer
{
public:
    ToneAnalyzer() = default;
    ~ToneAnalyzer()
    {
        // Ensure background thread is done before destruction
        if (analysisThread && analysisThread->joinable())
            analysisThread->join();
    }

    // Analyze audio file and return tone profile
    ToneProfile analyzeFile(const juce::File& audioFile);

    // Apply tone profile to APVTS (sets effect parameters)
    void applyProfile(const ToneProfile& profile, juce::AudioProcessorValueTreeState& apvts);

    // Last analyzed profile
    ToneProfile lastProfile;
    std::atomic<bool> analysisComplete{ false };
    std::atomic<float> progress{ 0.f };
    std::atomic<bool> isAnalyzing{ false };

    void startAnalysis(const juce::File& file, juce::AudioProcessorValueTreeState& apvts);

private:
    std::unique_ptr<std::thread> analysisThread;

    float computeSpectralCentroid(const std::vector<float>& magnitudes, float binHz) const;
    float computeBandEnergy(const std::vector<float>& magnitudes, float binHz, float lowHz, float highHz) const;
    float computeTHD(const std::vector<float>& magnitudes, float fundamentalHz, float binHz) const;
    float computeStereoWidth(juce::AudioFormatReader* reader) const;
};
