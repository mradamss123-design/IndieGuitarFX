#pragma once
#include <JuceHeader.h>

class IndieLookAndFeel : public juce::LookAndFeel_V4
{
public:
    IndieLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonWidth, int buttonHeight,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    juce::Font getLabelFont(juce::Label&) override;

    // Colour palette
    static juce::Colour bgDark()     { return juce::Colour(0xff1a1a2e); }
    static juce::Colour bgMid()      { return juce::Colour(0xff16213e); }
    static juce::Colour bgLight()    { return juce::Colour(0xff0f3460); }
    static juce::Colour accent()     { return juce::Colour(0xffe94560); }
    static juce::Colour accentSoft() { return juce::Colour(0xff533483); }
    static juce::Colour textBright() { return juce::Colour(0xfff5f5f5); }
    static juce::Colour textDim()    { return juce::Colour(0xff8888aa); }
    static juce::Colour green()      { return juce::Colour(0xff00d4aa); }
};
