#include "EffectStrip.h"

EffectStrip::EffectStrip(const juce::String& name,
                         juce::AudioProcessorValueTreeState& v,
                         const juce::String& bypassParamId,
                         juce::Colour accentColour)
    : effectName(name), accent(accentColour), apvts(v)
{
    bypassButton.setButtonText("ON");
    addAndMakeVisible(bypassButton);

    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, bypassParamId, bypassButton);
}

EffectStrip::~EffectStrip()
{
    bypassAttach.reset();

    for (auto& k : knobs)
    {
        k.attach.reset();
        k.slider.reset();
        k.label.reset();
    }
    for (auto& c : combos)
    {
        c.attach.reset();
        c.box.reset();
        c.label.reset();
    }
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

    addAndMakeVisible(*item.slider);
    addAndMakeVisible(*item.label);

    item.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramId, *item.slider);

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

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramId)))
        for (int i = 0; i < param->choices.size(); ++i)
            item.box->addItem(param->choices[i], i + 1);

    addAndMakeVisible(*item.box);
    addAndMakeVisible(*item.label);

    item.attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, paramId, *item.box);

    combos.push_back(std::move(item));
}

void EffectStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(IndieLookAndFeel::bgMid());
    g.fillRoundedRectangle(bounds, 8.f);

    g.setColour(accent);
    g.fillRoundedRectangle(bounds.withHeight(3.f), 2.f);

    g.setColour(IndieLookAndFeel::bgLight().withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.f, 1.f);

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
        int cW = numCombos > 0 ? availW / numCombos : availW;
        c.label->setBounds(x, comboY, cW, labelH);
        c.box->setBounds(x, comboY + labelH + 2, cW - margin, comboH);
        x += cW;
    }
}
