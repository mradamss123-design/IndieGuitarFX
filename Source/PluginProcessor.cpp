#include "PluginProcessor.h"
#include "PluginEditor.h"

IndieGuitarFXAudioProcessor::IndieGuitarFXAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

IndieGuitarFXAudioProcessor::~IndieGuitarFXAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout IndieGuitarFXAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ---- Compressor ----
    params.push_back(std::make_unique<juce::AudioParameterBool>("comp_on", "Compressor On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("comp_threshold", "Comp Threshold", -60.f, 0.f, -24.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("comp_ratio", "Comp Ratio", 1.f, 20.f, 4.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("comp_attack", "Comp Attack", 0.1f, 100.f, 10.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("comp_release", "Comp Release", 10.f, 1000.f, 100.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("comp_gain", "Comp Gain", -12.f, 12.f, 3.f));

    // ---- EQ ----
    params.push_back(std::make_unique<juce::AudioParameterBool>("eq_on", "EQ On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_low", "EQ Low", -12.f, 12.f, 0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_lowmid", "EQ LowMid", -12.f, 12.f, 2.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_highmid", "EQ HighMid", -12.f, 12.f, 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_high", "EQ High", -12.f, 12.f, 0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_low_freq", "EQ Low Freq", 60.f, 300.f, 100.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_lowmid_freq", "EQ LowMid Freq", 200.f, 800.f, 400.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_highmid_freq", "EQ HighMid Freq", 800.f, 4000.f, 2000.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("eq_high_freq", "EQ High Freq", 3000.f, 12000.f, 8000.f));

    // ---- Overdrive ----
    params.push_back(std::make_unique<juce::AudioParameterBool>("od_on", "Overdrive On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("od_drive", "Drive", 0.f, 1.f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("od_tone", "Tone", 0.f, 1.f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("od_level", "Level", 0.f, 1.f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("od_type", "OD Type",
        juce::StringArray{"Tube Screamer", "Blues Driver", "Fuzz"}, 0));

    // ---- Chorus ----
    params.push_back(std::make_unique<juce::AudioParameterBool>("ch_on", "Chorus On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ch_rate", "Chorus Rate", 0.1f, 5.f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ch_depth", "Chorus Depth", 0.f, 1.f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ch_mix", "Chorus Mix", 0.f, 1.f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ch_feedback", "Chorus Feedback", 0.f, 0.9f, 0.2f));

    // ---- Delay ----
    params.push_back(std::make_unique<juce::AudioParameterBool>("del_on", "Delay On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("del_time", "Delay Time", 50.f, 1000.f, 375.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("del_feedback", "Delay Feedback", 0.f, 0.95f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("del_mix", "Delay Mix", 0.f, 1.f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("del_sync", "Delay Sync", false));

    // ---- Reverb ----
    params.push_back(std::make_unique<juce::AudioParameterBool>("rev_on", "Reverb On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("rev_size", "Reverb Size", 0.f, 1.f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("rev_damping", "Reverb Damping", 0.f, 1.f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("rev_width", "Reverb Width", 0.f, 1.f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("rev_mix", "Reverb Mix", 0.f, 1.f, 0.25f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("rev_type", "Reverb Type",
        juce::StringArray{"Room", "Hall", "Plate", "Spring"}, 0));

    // ---- Master ----
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master_vol", "Master Volume", 0.f, 2.f, 1.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master_pan", "Pan", -1.f, 1.f, 0.f));

    return { params.begin(), params.end() };
}

void IndieGuitarFXAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)samplesPerBlock, 2 };

    compressor.prepare(spec, apvts);
    eq.prepare(spec, apvts);
    overdrive.prepare(spec, apvts);
    chorus.prepare(spec, apvts);
    delay.prepare(spec, apvts);
    reverb.prepare(spec, apvts);

    fftBuffer.fill(0.f);
    fftWritePos = 0;
}

void IndieGuitarFXAudioProcessor::releaseResources() {}

bool IndieGuitarFXAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void IndieGuitarFXAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto* compOn   = apvts.getRawParameterValue("comp_on");
    auto* eqOn     = apvts.getRawParameterValue("eq_on");
    auto* odOn     = apvts.getRawParameterValue("od_on");
    auto* chOn     = apvts.getRawParameterValue("ch_on");
    auto* delOn    = apvts.getRawParameterValue("del_on");
    auto* revOn    = apvts.getRawParameterValue("rev_on");
    auto* masterVol = apvts.getRawParameterValue("master_vol");
    auto* masterPan = apvts.getRawParameterValue("master_pan");

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    // Signal chain: Comp -> EQ -> OD -> Chorus -> Delay -> Reverb
    if (*compOn > 0.5f) compressor.process(ctx, apvts);
    if (*eqOn   > 0.5f) eq.process(ctx, apvts);
    if (*odOn   > 0.5f) overdrive.process(ctx, apvts);
    if (*chOn   > 0.5f) chorus.process(ctx, apvts);
    if (*delOn  > 0.5f) delay.process(ctx, apvts);
    if (*revOn  > 0.5f) reverb.process(ctx, apvts);

    // Master volume + pan
    float vol = *masterVol;
    float pan = *masterPan;
    float leftGain  = vol * (pan <= 0.f ? 1.f : 1.f - pan);
    float rightGain = vol * (pan >= 0.f ? 1.f : 1.f + pan);

    if (buffer.getNumChannels() >= 2)
    {
        buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
        buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);
    }
    else
    {
        buffer.applyGain(vol);
    }

    // FFT feed for spectrum display (use left channel)
    auto* readPtr = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        fftBuffer[(size_t)fftWritePos] = readPtr[i];
        fftWritePos = (fftWritePos + 1) % 1024;

        if (fftWritePos == 0)
        {
            auto bufCopy = fftBuffer;
            fftWindow.multiplyWithWindowingTable(bufCopy.data(), 1024);
            fft.performFrequencyOnlyForwardTransform(bufCopy.data());
            {
                juce::SpinLock::ScopedLockType lock(fftLock);
                for (int k = 0; k < 512; ++k)
                    fftData[(size_t)k] = bufCopy[(size_t)k];
            }
            fftDataReady = true;
        }
    }
}

void IndieGuitarFXAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void IndieGuitarFXAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* IndieGuitarFXAudioProcessor::createEditor()
{
    return new IndieGuitarFXAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IndieGuitarFXAudioProcessor();
}
