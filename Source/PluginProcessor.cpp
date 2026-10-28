#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace twixi;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TwixiAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    const StringArray types   { "Bell", "Low Shelf", "High Shelf", "Low Cut", "High Cut", "Notch" };
    const StringArray slopes  { "12 dB/oct", "24 dB/oct", "48 dB/oct" };
    const StringArray chans   { "Stereo", "Mid", "Side" };

    const bool  defOn[]   = { false, true, true, true, true, false };
    const int   defType[] = { LowCut, LowShelf, Bell, Bell, HighShelf, HighCut };
    const float defFreq[] = { 30.f, 100.f, 500.f, 2000.f, 8000.f, 18000.f };

    auto freqText = [] (float v, int) -> String
    {
        return v >= 1000.f ? String (v / 1000.f, 2) + " kHz" : String ((int) std::round (v)) + " Hz";
    };
    auto freqFromText = [] (const String& t) -> float
    {
        auto s = t.trim().toLowerCase();
        float v = s.getFloatValue();
        if (s.contains ("k")) v *= 1000.f;
        return v;
    };
    auto dbText = [] (float v, int) -> String
    {
        return String (v > 0.05f ? "+" : "") + String (v, 1) + " dB";
    };
    auto dbFromText = [] (const String& t) { return t.trim().getFloatValue(); };

    for (int i = 0; i < kNumBands; ++i)
    {
        const String n = "Band " + String (i + 1) + " ";

        layout.add (std::make_unique<AudioParameterBool> (ParameterID { bandParamId (i, "on"), 1 }, n + "On", defOn[i]));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { bandParamId (i, "type"), 1 }, n + "Type", types, defType[i]));

        NormalisableRange<float> fr (20.f, 20000.f, 0.f);
        fr.setSkewForCentre (1000.f);
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { bandParamId (i, "freq"), 1 }, n + "Freq", fr, defFreq[i],
                    AudioParameterFloatAttributes().withStringFromValueFunction (freqText).withValueFromStringFunction (freqFromText)));

        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { bandParamId (i, "gain"), 1 }, n + "Gain",
                    NormalisableRange<float> (-24.f, 24.f, 0.1f), 0.f,
                    AudioParameterFloatAttributes().withStringFromValueFunction (dbText).withValueFromStringFunction (dbFromText)));

        NormalisableRange<float> qr (0.1f, 18.f, 0.f);
        qr.setSkewForCentre (1.f);
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { bandParamId (i, "q"), 1 }, n + "Q", qr, 0.71f,
                    AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v, 2); })));

        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { bandParamId (i, "slope"), 1 }, n + "Slope", slopes, 1));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { bandParamId (i, "ch"), 1 }, n + "Channel", chans, 0));
    }

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "out_gain", 1 }, "Output",
                NormalisableRange<float> (-24.f, 24.f, 0.1f), 0.f,
                AudioParameterFloatAttributes().withStringFromValueFunction (dbText).withValueFromStringFunction (dbFromText)));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "range", 1 }, "Display Range",
                StringArray { "+/-6 dB", "+/-12 dB", "+/-18 dB", "+/-24 dB" }, 2));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "analyzer", 1 }, "Analyzer", true));

    return layout;
}

//==============================================================================
TwixiAudioProcessor::TwixiAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    for (int i = 0; i < kNumBands; ++i)
    {
        auto& p = params[(size_t) i];
        p.on      = apvts.getRawParameterValue (bandParamId (i, "on"));
        p.type    = apvts.getRawParameterValue (bandParamId (i, "type"));
        p.freq    = apvts.getRawParameterValue (bandParamId (i, "freq"));
        p.gain    = apvts.getRawParameterValue (bandParamId (i, "gain"));
        p.q       = apvts.getRawParameterValue (bandParamId (i, "q"));
        p.slope   = apvts.getRawParameterValue (bandParamId (i, "slope"));
        p.channel = apvts.getRawParameterValue (bandParamId (i, "ch"));
    }
    outGainPtr  = apvts.getRawParameterValue ("out_gain");
    rangePtr    = apvts.getRawParameterValue ("range");
    analyzerPtr = apvts.getRawParameterValue ("analyzer");
}

BandSettings TwixiAudioProcessor::getBandSettings (int band) const
{
    const auto& p = params[(size_t) band];
    BandSettings s;
    s.on      = p.on->load() > 0.5f;
    s.type    = (int) p.type->load();
    s.freq    = p.freq->load();
    s.gain    = p.gain->load();
    s.q       = p.q->load();
    s.slope   = (int) p.slope->load();
    s.channel = (int) p.channel->load();
    return s;
}

float TwixiAudioProcessor::getRangeDb() const
{
    static const float v[] = { 6.f, 12.f, 18.f, 24.f };
    return v[juce::jlimit (0, 3, (int) rangePtr->load())];
}

void TwixiAudioProcessor::copyAnalyzerBlock (float* dest) const
{
    const int end = ringPos.load();
    for (int i = 0; i < fftSize; ++i)
        dest[i] = ring[(size_t) ((end - fftSize + i) & (ringSize - 1))];
}

//==============================================================================
void TwixiAudioProcessor::prepareToPlay (double sr, int)
{
    sampleRateHz = sr;
    outSmooth.reset (sr, 0.05);
    outSmooth.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outGainPtr->load()));

    for (int b = 0; b < kNumBands; ++b)
    {
        cached[b].type = -1;          // fuerza recalculo de coeficientes
        numStages[b] = 0;
        for (auto& st : stages[b]) st = {};
    }
}

bool TwixiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

double TwixiAudioProcessor::runBand (int band, int ch, double x) noexcept
{
    for (int k = 0; k < numStages[band]; ++k)
    {
        auto& st = stages[band][k];
        const double y = st.c.b0 * x + st.z1[ch];
        st.z1[ch] = st.c.b1 * x - st.c.a1 * y + st.z2[ch];
        st.z2[ch] = st.c.b2 * x - st.c.a2 * y;
        x = y;
    }
    return x;
}

void TwixiAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, n);

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    if (numCh == 0) return;

    float* L = buffer.getWritePointer (0);
    float* R = numCh > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int b = 0; b < kNumBands; ++b)
    {
        const auto s = getBandSettings (b);

        if (! (s == cached[b]))
        {
            BiquadCoeffs co[kMaxStages];
            const int ns = designBand (s, sampleRateHz, co);

            const bool resetState = (s.on && ! cached[b].on) || s.channel != cached[b].channel
                                    || s.type != cached[b].type || ns != numStages[b];
            for (int k = 0; k < ns; ++k)
            {
                stages[b][k].c = co[k];
                if (resetState) { stages[b][k].z1[0] = stages[b][k].z1[1] = 0; stages[b][k].z2[0] = stages[b][k].z2[1] = 0; }
            }
            numStages[b] = ns;
            cached[b] = s;
        }

        if (! s.on) continue;
        if (s.hasGain() && std::abs (s.gain) < 0.005f) continue;

        if (R == nullptr)
        {
            for (int i = 0; i < n; ++i) L[i] = (float) runBand (b, 0, L[i]);
        }
        else if (s.channel == Stereo)
        {
            for (int i = 0; i < n; ++i)
            {
                L[i] = (float) runBand (b, 0, L[i]);
                R[i] = (float) runBand (b, 1, R[i]);
            }
        }
        else
        {
            const bool mid = (s.channel == Mid);
            for (int i = 0; i < n; ++i)
            {
                double m = 0.5 * (L[i] + R[i]);
                double sd = 0.5 * (L[i] - R[i]);
                if (mid) m = runBand (b, 0, m); else sd = runBand (b, 1, sd);
                L[i] = (float) (m + sd);
                R[i] = (float) (m - sd);
            }
        }
    }

    // ganancia de salida suavizada
    outSmooth.setTargetValue (juce::Decibels::decibelsToGain (outGainPtr->load()));
    for (int i = 0; i < n; ++i)
    {
        const float g = outSmooth.getNextValue();
        L[i] *= g;
        if (R) R[i] *= g;
    }

    // analizador de espectro (post-EQ)
    if (isAnalyzerOn())
    {
        int pos = ringPos.load();
        for (int i = 0; i < n; ++i)
        {
            ring[(size_t) (pos & (ringSize - 1))] = R ? 0.5f * (L[i] + R[i]) : L[i];
            ++pos;
        }
        ringPos.store (pos);
    }
}

//==============================================================================
juce::AudioProcessorEditor* TwixiAudioProcessor::createEditor() { return new TwixiEditor (*this); }

void TwixiAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TwixiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TwixiAudioProcessor(); }
