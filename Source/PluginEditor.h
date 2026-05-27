#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/KnobLookAndFeel.h"
#include "GUI/EffectStrip.h"
#include "GUI/ToneAnalyzerPanel.h"
#include "GUI/SpectrumAnalyzer.h"

class IndieGuitarFXAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           public juce::Timer
{
public:
    IndieGuitarFXAudioProcessorEditor(IndieGuitarFXAudioProcessor&);
    ~IndieGuitarFXAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    IndieGuitarFXAudioProcessor& audioProcessor;
    IndieLookAndFeel globalLAF;

    // Spectrum analyzer
    SpectrumAnalyzer spectrumAnalyzer;

    // Effect strips
    EffectStrip compStrip;
    EffectStrip eqStrip;
    EffectStrip overdriveStrip;
    EffectStrip chorusStrip;
    EffectStrip delayStrip;
    EffectStrip reverbStrip;

    // Master section
    juce::Slider masterVolSlider;
    juce::Label  masterVolLabel;
    juce::Slider masterPanSlider;
    juce::Label  masterPanLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterPanAttach;

    // Tone Analyzer
    ToneAnalyzerPanel tonePanel;

    // Preset tabs label
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IndieGuitarFXAudioProcessorEditor)
};
