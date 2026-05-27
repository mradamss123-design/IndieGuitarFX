#pragma once
#include <JuceHeader.h>
#include "KnobLookAndFeel.h"

class EffectStrip : public juce::Component
{
public:
    EffectStrip(const juce::String& name,
                juce::AudioProcessorValueTreeState& apvts,
                const juce::String& bypassParamId,
                juce::Colour accentColour = IndieLookAndFeel::accent());
    ~EffectStrip() override;

    void addKnob(const juce::String& paramId, const juce::String& label);
    void addComboBox(const juce::String& paramId, const juce::String& label);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::String effectName;
    juce::Colour accent;

    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    struct KnobItem
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    struct ComboItem
    {
        std::unique_ptr<juce::ComboBox> box;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attach;
    };

    std::vector<KnobItem> knobs;
    std::vector<ComboItem> combos;

    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectStrip)
};
