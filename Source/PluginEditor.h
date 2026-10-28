#pragma once
#include "PluginProcessor.h"

//==============================================================================
// Paleta de las bandas (neon, estilo alien)
inline juce::Colour bandColour (int i)
{
    static const juce::uint32 c[] = { 0xffb6ff3b, 0xffffe14a, 0xffa77bff, 0xff3ee0ff, 0xffff6ec7, 0xffffa63d };
    return juce::Colour (c[i % 6]);
}

//==============================================================================
class TwixiLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TwixiLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (12.f, juce::Font::bold); }
    juce::Font getPopupMenuFont() override { return juce::Font (13.f, juce::Font::bold); }
    juce::Font getLabelFont (juce::Label&) override { return juce::Font (11.f, juce::Font::bold); }
};

//==============================================================================
// Grafico: analizador + curva de respuesta + nodos arrastrables
class EqGraph : public juce::Component, private juce::Timer
{
public:
    explicit EqGraph (TwixiAudioProcessor&);
    ~EqGraph() override { stopTimer(); }

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { hoverBand = -1; repaint(); }
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override;
    float freqToX (float f) const;
    float xToFreq (float x) const;
    float gainToY (float db, float range) const;
    float yToGain (float y, float range) const;
    juce::Point<float> nodePos (const twixi::BandSettings&, float range) const;
    int hitTest (juce::Point<float>) const;
    void setParam (const juce::String& id, float value);
    void setParamWithGesture (const juce::String& id, float value);

    TwixiAudioProcessor& proc;
    juce::Rectangle<float> plot;
    int hoverBand = -1, dragBand = -1;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, 2 * TwixiAudioProcessor::fftSize> fftData {};
    std::array<float, TwixiAudioProcessor::fftSize / 2> smooth {};
};

//==============================================================================
class TwixiEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TwixiEditor (TwixiAudioProcessor&);
    ~TwixiEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct BandUI
    {
        juce::ToggleButton on;
        juce::ComboBox type, slope, channel;
        juce::Slider freq, gain, q;
        std::unique_ptr<ButtonAtt> onA;
        std::unique_ptr<ComboAtt> typeA, slopeA, channelA;
        std::unique_ptr<SliderAtt> freqA, gainA, qA;
        int lastType = -1;
        bool lastOn = false;
    };

    void timerCallback() override;
    void renderBackground();
    void initKnob (juce::Slider&, juce::Colour);
    juce::StringArray choicesOf (const juce::String& id);
    juce::Rectangle<int> panelBounds (int band) const;

    TwixiAudioProcessor& proc;
    TwixiLookAndFeel laf;
    EqGraph graph;
    juce::Image background;
    std::array<BandUI, twixi::kNumBands> bands;

    juce::Slider outGain;
    juce::ComboBox rangeBox;
    juce::ToggleButton analyzerBtn { "ANALYZER" };
    std::unique_ptr<SliderAtt> outGainA;
    std::unique_ptr<ComboAtt> rangeA;
    std::unique_ptr<ButtonAtt> analyzerA;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwixiEditor)
};
