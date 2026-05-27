#pragma once
#include <JuceHeader.h>
#include "DSP/Overdrive.h"
#include "DSP/Chorus.h"
#include "DSP/Reverb.h"
#include "DSP/Delay.h"
#include "DSP/Compressor.h"
#include "DSP/EQ.h"
#include "DSP/ToneAnalyzer.h"

class IndieGuitarFXAudioProcessor : public juce::AudioProcessor
{
public:
    IndieGuitarFXAudioProcessor();
    ~IndieGuitarFXAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    ToneAnalyzer toneAnalyzer;

    // FFT data for spectrum display — protected by spinLock
    std::array<float, 512> fftData{};
    std::atomic<bool> fftDataReady{ false };
    juce::SpinLock fftLock;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    Overdrive overdrive;
    Chorus chorus;
    GuitarReverb reverb;
    GuitarDelay delay;
    GuitarCompressor compressor;
    GuitarEQ eq;

    juce::dsp::FFT fft{ 10 };
    juce::dsp::WindowingFunction<float> fftWindow{ 1024, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, 1024> fftBuffer{};
    int fftWritePos = 0;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IndieGuitarFXAudioProcessor)
};
