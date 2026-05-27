#include "PluginEditor.h"

IndieGuitarFXAudioProcessorEditor::IndieGuitarFXAudioProcessorEditor(IndieGuitarFXAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      compStrip     ("Compressor", p.apvts, "comp_on",  juce::Colour(0xff00d4aa)),
      eqStrip       ("EQ 4-Band",  p.apvts, "eq_on",   juce::Colour(0xff4a9eff)),
      overdriveStrip("Overdrive",  p.apvts, "od_on",   juce::Colour(0xffe94560)),
      chorusStrip   ("Chorus",     p.apvts, "ch_on",   juce::Colour(0xffb060e0)),
      delayStrip    ("Delay",      p.apvts, "del_on",  juce::Colour(0xffffaa00)),
      reverbStrip   ("Reverb",     p.apvts, "rev_on",  juce::Colour(0xff60c0ff)),
      tonePanel     (p.toneAnalyzer, p.apvts)
{
    setLookAndFeel(&globalLAF);

    // --- Compressor knobs ---
    compStrip.addKnob("comp_threshold", "Thresh");
    compStrip.addKnob("comp_ratio",     "Ratio");
    compStrip.addKnob("comp_attack",    "Attack");
    compStrip.addKnob("comp_release",   "Release");
    compStrip.addKnob("comp_gain",      "Gain");

    // --- EQ knobs ---
    eqStrip.addKnob("eq_low",     "Low");
    eqStrip.addKnob("eq_lowmid",  "Low Mid");
    eqStrip.addKnob("eq_highmid", "Hi Mid");
    eqStrip.addKnob("eq_high",    "High");

    // --- Overdrive knobs ---
    overdriveStrip.addKnob("od_drive", "Drive");
    overdriveStrip.addKnob("od_tone",  "Tone");
    overdriveStrip.addKnob("od_level", "Level");
    overdriveStrip.addComboBox("od_type", "Type");

    // --- Chorus knobs ---
    chorusStrip.addKnob("ch_rate",     "Rate");
    chorusStrip.addKnob("ch_depth",    "Depth");
    chorusStrip.addKnob("ch_mix",      "Mix");
    chorusStrip.addKnob("ch_feedback", "Feedback");

    // --- Delay knobs ---
    delayStrip.addKnob("del_time",     "Time ms");
    delayStrip.addKnob("del_feedback", "Feedback");
    delayStrip.addKnob("del_mix",      "Mix");

    // --- Reverb knobs ---
    reverbStrip.addKnob("rev_size",    "Size");
    reverbStrip.addKnob("rev_damping", "Damping");
    reverbStrip.addKnob("rev_width",   "Width");
    reverbStrip.addKnob("rev_mix",     "Mix");
    reverbStrip.addComboBox("rev_type", "Type");

    // Add all strips
    addAndMakeVisible(spectrumAnalyzer);
    addAndMakeVisible(compStrip);
    addAndMakeVisible(eqStrip);
    addAndMakeVisible(overdriveStrip);
    addAndMakeVisible(chorusStrip);
    addAndMakeVisible(delayStrip);
    addAndMakeVisible(reverbStrip);
    addAndMakeVisible(tonePanel);

    // --- Master ---
    masterVolSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    masterVolSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    masterPanSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    masterPanSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    masterVolLabel.setText("VOL", juce::dontSendNotification);
    masterVolLabel.setJustificationType(juce::Justification::centred);
    masterVolLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());
    masterVolLabel.setFont(juce::Font(9.5f, juce::Font::bold));

    masterPanLabel.setText("PAN", juce::dontSendNotification);
    masterPanLabel.setJustificationType(juce::Justification::centred);
    masterPanLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());
    masterPanLabel.setFont(juce::Font(9.5f, juce::Font::bold));

    masterVolAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "master_vol", masterVolSlider);
    masterPanAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "master_pan", masterPanSlider);

    addAndMakeVisible(masterVolSlider);
    addAndMakeVisible(masterVolLabel);
    addAndMakeVisible(masterPanSlider);
    addAndMakeVisible(masterPanLabel);

    // --- Title ---
    titleLabel.setText("INDIE GUITAR FX", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::textBright());
    titleLabel.setJustificationType(juce::Justification::centredLeft);

    subtitleLabel.setText("VST3 | Indie Rock / Pop Channel FX", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(9.f));
    subtitleLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(subtitleLabel);

    setSize(960, 560);
    startTimerHz(30);
}

IndieGuitarFXAudioProcessorEditor::~IndieGuitarFXAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void IndieGuitarFXAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.fftDataReady.exchange(false))
    {
        std::array<float, 512> copy;
        {
            juce::SpinLock::ScopedLockType lock(audioProcessor.fftLock);
            copy = audioProcessor.fftData;
        }
        spectrumAnalyzer.setFFTData(copy);
    }
}

void IndieGuitarFXAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark gradient background
    juce::ColourGradient bg(
        IndieLookAndFeel::bgDark(),  0.f, 0.f,
        IndieLookAndFeel::bgMid().darker(0.3f), (float)getWidth(), (float)getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillAll();

    // Horizontal separator under header
    g.setColour(IndieLookAndFeel::bgLight().withAlpha(0.5f));
    g.drawHorizontalLine(50, 0.f, (float)getWidth());

    // Subtle scanline texture
    g.setColour(juce::Colours::black.withAlpha(0.03f));
    for (int y = 0; y < getHeight(); y += 2)
        g.drawHorizontalLine(y, 0.f, (float)getWidth());
}

void IndieGuitarFXAudioProcessorEditor::resized()
{
    const int W  = getWidth();
    const int H  = getHeight();
    const int hdr = 52;
    const int pad = 6;
    const int specH = 70;
    const int masterW = 110;
    const int analyzerW = 200;

    // Header
    titleLabel.setBounds(12, 8, 300, 22);
    subtitleLabel.setBounds(12, 30, 300, 14);

    // Master knobs (top right)
    masterVolSlider.setBounds(W - masterW - 4, 4, 48, 48);
    masterVolLabel .setBounds(W - masterW - 4, 48, 48, 12);
    masterPanSlider.setBounds(W - masterW + 56, 4, 48, 48);
    masterPanLabel .setBounds(W - masterW + 56, 48, 48, 12);

    // Spectrum analyzer (top center)
    int specX = 320;
    int specW = W - specX - masterW - 16;
    spectrumAnalyzer.setBounds(specX, 6, specW, specH - 8);

    // Effect strips (main area)
    int stripAreaY = hdr + pad;
    int stripAreaH = H - stripAreaY - pad;
    int stripAreaW = W - analyzerW - pad * 2;

    // 6 strips in 2 rows of 3
    int cols = 3, rows = 2;
    int stripW = (stripAreaW - pad * (cols - 1)) / cols;
    int stripH = (stripAreaH - pad * (rows - 1)) / rows;

    juce::Component* strips[] = {
        &compStrip, &eqStrip, &overdriveStrip,
        &chorusStrip, &delayStrip, &reverbStrip
    };

    for (int i = 0; i < 6; ++i)
    {
        int col = i % cols;
        int row = i / cols;
        int x = pad + col * (stripW + pad);
        int y = stripAreaY + row * (stripH + pad);
        strips[i]->setBounds(x, y, stripW, stripH);
    }

    // Tone analyzer panel (right side)
    tonePanel.setBounds(W - analyzerW - pad, stripAreaY, analyzerW, stripAreaH);
}
