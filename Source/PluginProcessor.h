#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include "EQEngine.h"

inline juce::String bandParamId (int band, const char* suffix)
{
    return "b" + juce::String (band + 1) + "_" + suffix;
}

class TwixiAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int fftOrder = 12;
    static constexpr int fftSize  = 1 << fftOrder;

    TwixiAudioProcessor();
    ~TwixiAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Twixi-6 EQ"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // acceso para la interfaz
    twixi::BandSettings getBandSettings (int band) const;
    float getRangeDb() const;
    bool  isAnalyzerOn() const { return analyzerPtr->load() > 0.5f; }
    void  copyAnalyzerBlock (float* dest) const;   // copia fftSize muestras

    juce::AudioProcessorValueTreeState apvts;

private:
    struct BandParams
    {
        std::atomic<float> *on, *type, *freq, *gain, *q, *slope, *channel;
    };
    struct Stage
    {
        twixi::BiquadCoeffs c;
        double z1[2] { 0, 0 }, z2[2] { 0, 0 };
    };

    double runBand (int band, int ch, double x) noexcept;

    std::array<BandParams, twixi::kNumBands> params {};
    std::atomic<float> *outGainPtr = nullptr, *rangePtr = nullptr, *analyzerPtr = nullptr;

    Stage stages[twixi::kNumBands][twixi::kMaxStages];
    int numStages[twixi::kNumBands] {};
    twixi::BandSettings cached[twixi::kNumBands];

    juce::SmoothedValue<float> outSmooth;
    double sampleRateHz = 44100.0;

    static constexpr int ringSize = 16384;
    std::array<float, ringSize> ring {};
    std::atomic<int> ringPos { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwixiAudioProcessor)
};
