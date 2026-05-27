#include "ToneAnalyzerPanel.h"
#include "KnobLookAndFeel.h"

ToneAnalyzerPanel::ToneAnalyzerPanel(ToneAnalyzer& a, juce::AudioProcessorValueTreeState& v)
    : analyzer(a), apvts(v)
{
    addAndMakeVisible(loadButton);
    addAndMakeVisible(applyButton);
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(descriptionLabel);
    addAndMakeVisible(progressLabel);

    loadButton.setColour(juce::TextButton::buttonColourId, IndieLookAndFeel::bgLight());
    loadButton.setColour(juce::TextButton::textColourOnId, IndieLookAndFeel::textBright());
    loadButton.setColour(juce::TextButton::textColourOffId, IndieLookAndFeel::textBright());

    applyButton.setColour(juce::TextButton::buttonColourId, IndieLookAndFeel::accent());
    applyButton.setColour(juce::TextButton::textColourOnId, IndieLookAndFeel::textBright());
    applyButton.setColour(juce::TextButton::textColourOffId, IndieLookAndFeel::textBright());
    applyButton.setEnabled(false);

    statusLabel.setText("Drop an audio file here or click Load", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());
    statusLabel.setFont(juce::Font(11.f));
    statusLabel.setJustificationType(juce::Justification::centred);

    descriptionLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::accent());
    descriptionLabel.setFont(juce::Font(12.f, juce::Font::bold));
    descriptionLabel.setJustificationType(juce::Justification::centred);

    progressLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::green());
    progressLabel.setFont(juce::Font(10.f));
    progressLabel.setJustificationType(juce::Justification::centred);

    loadButton.onClick = [this] { openFileChooser(); };
    applyButton.onClick = [this]
    {
        if (analyzer.analysisComplete)
            analyzer.applyProfile(analyzer.lastProfile, apvts);
    };

    startTimerHz(10);
}

ToneAnalyzerPanel::~ToneAnalyzerPanel()
{
    stopTimer();
}

void ToneAnalyzerPanel::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select audio file to analyze",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.aiff;*.flac;*.ogg");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
                             juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.existsAsFile())
            {
                statusLabel.setText("Analyzing: " + result.getFileName(), juce::dontSendNotification);
                applyButton.setEnabled(false);
                analyzer.startAnalysis(result, apvts);
            }
        });
}

void ToneAnalyzerPanel::timerCallback()
{
    if (analyzer.isAnalyzing)
    {
        float p = analyzer.progress.load() * 100.f;
        progressLabel.setText(juce::String((int)p) + "%", juce::dontSendNotification);
        repaint();
    }
    else if (analyzer.analysisComplete)
    {
        progressLabel.setText("", juce::dontSendNotification);
        statusLabel.setText("Analysis complete!", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::green());
        descriptionLabel.setText(analyzer.lastProfile.description, juce::dontSendNotification);
        applyButton.setEnabled(true);
        repaint();
    }
}

bool ToneAnalyzerPanel::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& f : files)
    {
        juce::File file(f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".mp3" || ext == ".aiff" ||
            ext == ".flac" || ext == ".ogg") return true;
    }
    return false;
}

void ToneAnalyzerPanel::filesDropped(const juce::StringArray& files, int, int)
{
    isDraggingOver = false;
    repaint();
    if (files.isEmpty()) return;
    juce::File f(files[0]);
    if (f.existsAsFile())
    {
        statusLabel.setText("Analyzing: " + f.getFileName(), juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, IndieLookAndFeel::textDim());
        descriptionLabel.setText("", juce::dontSendNotification);
        applyButton.setEnabled(false);
        analyzer.startAnalysis(f, apvts);
    }
}

void ToneAnalyzerPanel::fileDragEnter(const juce::StringArray&, int, int)
{
    isDraggingOver = true;
    repaint();
}

void ToneAnalyzerPanel::fileDragExit(const juce::StringArray&)
{
    isDraggingOver = false;
    repaint();
}

void ToneAnalyzerPanel::drawProfileBars(juce::Graphics& g, const juce::Rectangle<float>& area)
{
    if (!analyzer.analysisComplete) return;

    const ToneProfile& p = analyzer.lastProfile;
    struct Bar { const char* name; float value; };
    Bar bars[] = {
        { "Bass",    p.bassAmount },
        { "Mid",     p.midAmount },
        { "Treble",  p.trebleAmount },
        { "Bright",  p.brightness },
        { "Drive",   p.harmonics },
        { "Reverb",  p.reverbAmount },
        { "Width",   p.stereoWidth },
    };
    int numBars = 7;
    float barW  = area.getWidth() / (float)numBars;
    float maxH  = area.getHeight() - 18.f;

    for (int i = 0; i < numBars; ++i)
    {
        float x = area.getX() + i * barW + 3.f;
        float barHeight = bars[i].value * maxH;
        float y = area.getBottom() - 18.f - barHeight;

        juce::ColourGradient barGrad(
            IndieLookAndFeel::accent(), x, y,
            IndieLookAndFeel::accentSoft(), x, area.getBottom() - 18.f, false);
        g.setGradientFill(barGrad);
        g.fillRoundedRectangle(x, y, barW - 6.f, barHeight, 3.f);

        g.setColour(IndieLookAndFeel::textDim());
        g.setFont(8.5f);
        g.drawFittedText(bars[i].name,
            (int)x, (int)(area.getBottom() - 16.f), (int)(barW - 4.f), 14,
            juce::Justification::centred, 1);
    }
}

void ToneAnalyzerPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Panel background
    g.setColour(IndieLookAndFeel::bgDark().darker(0.2f));
    g.fillRoundedRectangle(bounds, 8.f);

    // Drag highlight
    if (isDraggingOver)
    {
        g.setColour(IndieLookAndFeel::accent().withAlpha(0.2f));
        g.fillRoundedRectangle(bounds.reduced(2.f), 7.f);
        g.setColour(IndieLookAndFeel::accent());
        g.drawRoundedRectangle(bounds.reduced(2.f), 7.f, 2.f);
    }
    else
    {
        g.setColour(IndieLookAndFeel::bgLight().withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(1.f), 8.f, 1.f);
    }

    // Bars area
    auto barsArea = bounds.reduced(6.f).withTrimmedTop(16.f).withTrimmedBottom(70.f);
    drawProfileBars(g, barsArea);

    // Title
    g.setColour(IndieLookAndFeel::textDim());
    g.setFont(juce::Font(10.f, juce::Font::bold));
    g.drawText("TONE ANALYZER", bounds.withHeight(16.f).reduced(4.f, 2.f),
               juce::Justification::centredLeft);

    // Progress bar
    if (analyzer.isAnalyzing)
    {
        float p = analyzer.progress.load();
        auto progressArea = bounds.withTrimmedTop(bounds.getHeight() - 54.f)
                                  .withTrimmedBottom(30.f).reduced(8.f, 0.f);
        g.setColour(IndieLookAndFeel::bgLight());
        g.fillRoundedRectangle(progressArea, 3.f);
        g.setColour(IndieLookAndFeel::green());
        g.fillRoundedRectangle(progressArea.withWidth(progressArea.getWidth() * p), 3.f);
    }
}

void ToneAnalyzerPanel::resized()
{
    auto b = getLocalBounds().reduced(6);
    int btnY = b.getBottom() - 26;
    loadButton.setBounds(b.getX(), btnY, b.getWidth() / 2 - 2, 24);
    applyButton.setBounds(b.getX() + b.getWidth() / 2 + 2, btnY, b.getWidth() / 2 - 2, 24);
    statusLabel.setBounds(b.getX(), btnY - 28, b.getWidth(), 14);
    progressLabel.setBounds(b.getX(), btnY - 42, b.getWidth(), 14);
    descriptionLabel.setBounds(b.getX(), btnY - 14, b.getWidth(), 14);
}
