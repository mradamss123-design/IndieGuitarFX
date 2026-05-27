#include "EffectStrip.h"

EffectStrip::EffectStrip(const juce::String& name,
                         juce::AudioProcessorValueTreeState& v,
                         const juce::String& bypassParamId,
                         juce::Colour accentColour)
    : effectName(name), accent(accentColour), apvts(v)
{
    setLookAndFeel(&laf);

    bypassButton.setButtonText("ON");
    bypassButton.setToggleable(true);
    addAndMakeVisible(bypassButton);

    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, bypassParamId, bypassButton);
}

EffectStrip::~EffectStrip()
{
    // Must clear LAF before components are destroyed
    setLookAndFeel(nullptr);
    for (auto& k : knobs)   k.slider->setLookAndFeel(nullptr);
    for (auto& c : combos)  c.box->setLookAndFeel(nullptr);
}

void EffectStrip::addKnob(const juce::String& paramId, const juce::String& labelText)
{
    KnobItem item;
    item.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag,
                                                  juce::Slider::NoTextBox);
    item.label  = std::make_unique<juce::Label>("", labelText);
    item.label->setFont(juce::Font(9.5f, juce::Font::bold));
    item.label->setJustificationType(juce::Justification::centred);
    item.label->setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());

    item.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramId, *item.slider);

    addAndMakeVisible(*item.slider);
    addAndMakeVisible(*item.label);
    knobs.push_back(std::move(item));
}

void EffectStrip::addComboBox(const juce::String& paramId, const juce::String& labelText)
{
    ComboItem item;
    item.box   = std::make_unique<juce::ComboBox>();
    item.label = std::make_unique<juce::Label>("", labelText);
    item.label->setFont(juce::Font(9.5f, juce::Font::bold));
    item.label->setJustificationType(juce::Justification::centred);
    item.label->setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());

    // Populate from parameter choices
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramId)))
    {
        for (int i = 0; i < param->choices.size(); ++i)
            item.box->addItem(param->choices[i], i + 1);
    }

    item.attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, paramId, *item.box);

    addAndMakeVisible(*item.box);
    addAndMakeVisible(*item.label);
    combos.push_back(std::move(item));
}

void EffectStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(IndieLookAndFeel::bgMid());
    g.fillRoundedRectangle(bounds, 8.f);

    // Top accent bar
    g.setColour(accent);
    g.fillRoundedRectangle(bounds.withHeight(3.f), 2.f);

    // Border
    g.setColour(IndieLookAndFeel::bgLight().withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.f, 1.f);

    // Title
    g.setColour(IndieLookAndFeel::textBright());
    g.setFont(juce::Font(11.f, juce::Font::bold));
    g.drawText(effectName.toUpperCase(), 36, 5, getWidth() - 40, 16,
               juce::Justification::centredLeft);
}

void EffectStrip::resized()
{
    const int headerH = 26;
    const int knobSize = 52;
    const int labelH = 14;
    const int comboH = 22;
    const int margin = 4;

    bypassButton.setBounds(4, 6, 28, 16);

    // Layout: knobs first, then combos below
    int numKnobs  = (int)knobs.size();
    int numCombos = (int)combos.size();
    int totalItems = numKnobs + numCombos;
    if (totalItems == 0) return;

    int availW = getWidth() - margin * 2;
    int colW   = availW / totalItems;

    int x = margin;
    int y = headerH + 2;

    for (auto& k : knobs)
    {
        k.slider->setBounds(x + (colW - knobSize) / 2, y, knobSize, knobSize);
        k.label->setBounds(x, y + knobSize, colW, labelH);
        x += colW;
    }

    x = margin;
    int comboY = headerH + knobSize + labelH + 6;
    for (auto& c : combos)
    {
        c.label->setBounds(x, comboY, availW / numCombos, labelH);
        c.box->setBounds(x, comboY + labelH + 2, availW / numCombos - margin, comboH);
        x += availW / numCombos;
    }
}
