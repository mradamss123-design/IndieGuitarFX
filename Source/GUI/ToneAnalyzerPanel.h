#pragma once
#include <JuceHeader.h>
#include "../DSP/ToneAnalyzer.h"

class ToneAnalyzerPanel : public juce::Component,
                          public juce::Timer,
                          public juce::FileDragAndDropTarget
{
public:
    ToneAnalyzerPanel(ToneAnalyzer& analyzer, juce::AudioProcessorValueTreeState& apvts);
    ~ToneAnalyzerPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void visibilityChanged() override;

private:
    ToneAnalyzer& analyzer;
    juce::AudioProcessorValueTreeState& apvts;

    juce::TextButton loadButton{ "LOAD AUDIO FILE" };
    juce::TextButton applyButton{ "APPLY TONE" };

    juce::Label statusLabel;
    juce::Label descriptionLabel;
    juce::Label progressLabel;

    bool isDraggingOver = false;

    void drawProfileBars(juce::Graphics& g, const juce::Rectangle<float>& area);
    void openFileChooser();

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToneAnalyzerPanel)
};
