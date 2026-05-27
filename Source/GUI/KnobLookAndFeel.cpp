#include "KnobLookAndFeel.h"

IndieLookAndFeel::IndieLookAndFeel()
{
    setColour(juce::Slider::thumbColourId,              accent());
    setColour(juce::Slider::rotarySliderFillColourId,   accent());
    setColour(juce::Slider::rotarySliderOutlineColourId,bgLight());
    setColour(juce::Slider::trackColourId,              accentSoft());
    setColour(juce::Label::textColourId,                textBright());
    setColour(juce::ComboBox::backgroundColourId,       bgLight());
    setColour(juce::ComboBox::textColourId,             textBright());
    setColour(juce::ComboBox::arrowColourId,            accent());
    setColour(juce::ComboBox::outlineColourId,          accentSoft());
    setColour(juce::PopupMenu::backgroundColourId,      bgMid());
    setColour(juce::PopupMenu::textColourId,            textBright());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accentSoft());
    setColour(juce::ToggleButton::textColourId,         textBright());
    setColour(juce::ToggleButton::tickColourId,         accent());
    setColour(juce::ToggleButton::tickDisabledColourId, textDim());
}

void IndieLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius  = juce::jmin(width, height) * 0.5f - 4.f;
    const float centreX = (float)x + (float)width  * 0.5f;
    const float centreY = (float)y + (float)height * 0.5f;
    const float angle   = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Shadow
    {
        juce::ColourGradient shadow(juce::Colours::black.withAlpha(0.6f),
            centreX, centreY + radius,
            juce::Colours::transparentBlack,
            centreX, centreY - radius, true);
        g.setGradientFill(shadow);
        g.fillEllipse(centreX - radius - 2, centreY - radius + 4, radius * 2 + 4, radius * 2 + 4);
    }

    // Body gradient
    juce::ColourGradient bodyGrad(bgLight().brighter(0.15f),
        centreX - radius * 0.3f, centreY - radius * 0.3f,
        bgDark(),
        centreX + radius * 0.5f, centreY + radius * 0.5f, false);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

    // Outer ring
    g.setColour(bgLight().brighter(0.3f));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2, 1.2f);

    // Arc track
    juce::Path arcTrack;
    arcTrack.addArc(centreX - radius + 3, centreY - radius + 3,
                    (radius - 3) * 2, (radius - 3) * 2,
                    rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(bgLight().brighter(0.1f));
    g.strokePath(arcTrack, juce::PathStrokeType(2.f));

    // Filled arc
    juce::Path arcFill;
    arcFill.addArc(centreX - radius + 3, centreY - radius + 3,
                   (radius - 3) * 2, (radius - 3) * 2,
                   rotaryStartAngle, angle, true);
    g.setColour(accent());
    juce::PathStrokeType stroke(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    g.strokePath(arcFill, stroke);

    // Pointer line
    const float pointerLength = radius * 0.55f;
    const float pointerThickness = 2.5f;
    juce::Path pointer;
    pointer.addRectangle(-pointerThickness * 0.5f, -radius + 4.f, pointerThickness, pointerLength);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(textBright());
    g.fillPath(pointer);

    // Inner highlight dot
    g.setColour(accent().withAlpha(0.5f));
    g.fillEllipse(centreX - 3.f, centreY - 3.f, 6.f, 6.f);
}

void IndieLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool)
{
    const int h = button.getHeight();
    const int ledSize = 14;
    const int ledX = 6;
    const int ledY = (h - ledSize) / 2;

    juce::Colour ledColour = button.getToggleState() ? green() : textDim().darker(0.5f);
    if (shouldDrawButtonAsHighlighted) ledColour = ledColour.brighter(0.3f);

    // Glow
    if (button.getToggleState())
    {
        g.setColour(green().withAlpha(0.25f));
        g.fillEllipse((float)ledX - 3.f, (float)ledY - 3.f,
                      (float)ledSize + 6.f, (float)ledSize + 6.f);
    }

    g.setColour(bgDark().darker(0.2f));
    g.fillEllipse((float)ledX, (float)ledY, (float)ledSize, (float)ledSize);
    g.setColour(ledColour);
    g.fillEllipse((float)ledX + 2.f, (float)ledY + 2.f,
                  (float)ledSize - 4.f, (float)ledSize - 4.f);
    g.setColour(ledColour.brighter(0.4f));
    g.drawEllipse((float)ledX, (float)ledY, (float)ledSize, (float)ledSize, 1.2f);

    // Label
    g.setColour(findColour(juce::ToggleButton::textColourId));
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.drawFittedText(button.getButtonText(),
        ledX + ledSize + 4, 0, button.getWidth() - ledX - ledSize - 6, h,
        juce::Justification::centredLeft, 1);
}

void IndieLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
    bool, int, int, int, int, juce::ComboBox& box)
{
    g.setColour(bgLight());
    g.fillRoundedRectangle(0, 0, (float)width, (float)height, 5.f);
    g.setColour(accentSoft().withAlpha(0.6f));
    g.drawRoundedRectangle(0.5f, 0.5f, width - 1.f, height - 1.f, 5.f, 1.f);

    // Arrow
    const float arrowX = (float)width - 18.f;
    const float arrowY = (float)height * 0.5f - 3.f;
    juce::Path arrow;
    arrow.addTriangle(arrowX, arrowY, arrowX + 10.f, arrowY, arrowX + 5.f, arrowY + 6.f);
    g.setColour(accent());
    g.fillPath(arrow);
}

void IndieLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(bgMid());
    g.fillRoundedRectangle(0.f, 0.f, (float)width, (float)height, 6.f);
    g.setColour(accentSoft().withAlpha(0.5f));
    g.drawRoundedRectangle(0.5f, 0.5f, (float)width - 1.f, (float)height - 1.f, 6.f, 1.f);
}

juce::Font IndieLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(11.f, juce::Font::bold);
}
