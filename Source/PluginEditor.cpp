#include "PluginEditor.h"

using namespace twixi;

namespace
{
const juce::Colour kInk      { 0xff000000 };
const juce::Colour kPurple   { 0xff6d3fe0 };
const juce::Colour kNavy     { 0xff071545 };
const juce::Colour kYellow   { 0xffffe14a };
const juce::Colour kLime     { 0xffb6ff3b };

void drawOutlinedText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> r,
                       juce::Font f, juce::Colour fill, float outline,
                       juce::Justification j = juce::Justification::centredLeft)
{
    juce::GlyphArrangement ga;
    ga.addFittedText (f, text, r.getX(), r.getY(), r.getWidth(), r.getHeight(), j, 1);
    juce::Path p;
    ga.createPath (p);
    g.setColour (kInk);
    g.strokePath (p, juce::PathStrokeType (outline, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (fill);
    g.fillPath (p);
}

void drawStar (juce::Graphics& g, float cx, float cy, float r)
{
    juce::Path p;
    const float inner = r * 0.28f;
    for (int i = 0; i < 8; ++i)
    {
        const float a = juce::MathConstants<float>::pi * 0.25f * (float) i - juce::MathConstants<float>::halfPi;
        const float rad = (i % 2 == 0) ? r : inner;
        const float x = cx + std::cos (a) * rad, y = cy + std::sin (a) * rad;
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    p.closeSubPath();
    g.setColour (kInk);
    g.strokePath (p, juce::PathStrokeType (3.f, juce::PathStrokeType::mitered));
    g.setColour (kYellow);
    g.fillPath (p);
}

void drawCloud (juce::Graphics& g, float x, float y, float w)
{
    const float h = w * 0.4f;
    juce::Path p;
    p.addEllipse (x, y + 0.4f * h, 0.5f * w, 0.6f * h);
    p.addEllipse (x + 0.22f * w, y, 0.5f * w, 0.95f * h);
    p.addEllipse (x + 0.5f * w, y + 0.3f * h, 0.5f * w, 0.7f * h);
    g.setColour (kInk);
    g.strokePath (p, juce::PathStrokeType (7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff9b7bff));
    g.fillPath (p);
}

void drawEllipseOutlined (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour fill, float stroke)
{
    g.setColour (fill);
    g.fillEllipse (r);
    g.setColour (kInk);
    g.drawEllipse (r, stroke);
}

void drawUfo (juce::Graphics& g, float cx)
{
    using R = juce::Rectangle<float>;
    // domo
    drawEllipseOutlined (g, R (cx - 65.f, 8.f, 130.f, 90.f), juce::Colour (0xff3b46e8), 3.5f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.fillEllipse (cx - 50.f, 20.f, 22.f, 34.f);
    // alien
    drawEllipseOutlined (g, R (cx - 27.f, 24.f, 54.f, 52.f), juce::Colour (0xff8be85a), 3.f);
    drawEllipseOutlined (g, R (cx - 21.f, 40.f, 17.f, 22.f), kInk, 1.f);
    drawEllipseOutlined (g, R (cx + 4.f, 40.f, 17.f, 22.f), kInk, 1.f);
    g.setColour (juce::Colours::white);
    g.fillEllipse (cx - 15.f, 44.f, 5.f, 5.f);
    g.fillEllipse (cx + 10.f, 44.f, 5.f, 5.f);
    juce::Path smile;
    smile.startNewSubPath (cx - 8.f, 64.f);
    smile.quadraticTo (cx, 71.f, cx + 8.f, 64.f);
    g.setColour (kInk);
    g.strokePath (smile, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    // plato
    drawEllipseOutlined (g, R (cx - 150.f, 56.f, 300.f, 60.f), kPurple, 3.5f);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.fillEllipse (cx - 120.f, 62.f, 120.f, 14.f);
    // manitos del alien
    drawEllipseOutlined (g, R (cx - 36.f, 56.f, 18.f, 11.f), juce::Colour (0xff8be85a), 2.5f);
    drawEllipseOutlined (g, R (cx + 18.f, 56.f, 18.f, 11.f), juce::Colour (0xff8be85a), 2.5f);
    // luces
    const float lx[] = { -105.f, -52.f, 52.f, 105.f };
    const float ly[] = { 84.f, 91.f, 91.f, 84.f };
    for (int i = 0; i < 4; ++i)
        drawEllipseOutlined (g, R (cx + lx[i] - 17.f, ly[i], 34.f, 13.f), kYellow, 2.5f);
    // luz del rayo
    drawEllipseOutlined (g, R (cx - 45.f, 103.f, 90.f, 18.f), juce::Colour (0xffd8ff3a), 3.f);
}
} // namespace

//==============================================================================
TwixiLookAndFeel::TwixiLookAndFeel()
{
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0b1f6b));
    setColour (juce::PopupMenu::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kPurple);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, kPurple);
    setColour (juce::TextEditor::backgroundColourId, kNavy);
    setColour (juce::TextEditor::textColourId, juce::Colours::white);
    setColour (juce::TextEditor::outlineColourId, kPurple);
}

void TwixiLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const float r = std::min (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float ang = startAngle + pos * (endAngle - startAngle);
    const auto col = s.findColour (juce::Slider::rotarySliderFillColourId);
    const float tr = r - 3.f;

    juce::Path track;
    track.addCentredArc (c.x, c.y, tr, tr, 0.f, startAngle, endAngle, true);
    g.setColour (kInk);
    g.strokePath (track, juce::PathStrokeType (8.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff0a1650));
    g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;
    val.addCentredArc (c.x, c.y, tr, tr, 0.f, startAngle, ang, true);
    g.setColour (col);
    g.strokePath (val, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float kr = r - 9.f;
    g.setColour (kInk);
    g.fillEllipse (c.x - kr - 2.f, c.y - kr - 2.f, 2.f * kr + 4.f, 2.f * kr + 4.f);
    g.setColour (kPurple);
    g.fillEllipse (c.x - kr, c.y - kr, 2.f * kr, 2.f * kr);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.fillEllipse (c.x - kr * 0.6f, c.y - kr * 0.85f, kr * 0.8f, kr * 0.4f);

    const float dx = std::sin (ang), dy = -std::cos (ang);
    juce::Path ptr;
    ptr.startNewSubPath (c.x + dx * kr * 0.25f, c.y + dy * kr * 0.25f);
    ptr.lineTo (c.x + dx * kr * 0.92f, c.y + dy * kr * 0.92f);
    g.setColour (kInk);
    g.strokePath (ptr, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (col);
    g.strokePath (ptr, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void TwixiLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (1.5f);
    g.setColour (box.isEnabled() ? kPurple : juce::Colours::grey);
    g.fillRoundedRectangle (r, 7.f);
    g.setColour (kInk);
    g.drawRoundedRectangle (r, 7.f, 2.5f);
    juce::Path arrow;
    const float cx = (float) w - 12.f, cy = (float) h * 0.5f;
    arrow.addTriangle (cx - 4.f, cy - 2.f, cx + 4.f, cy - 2.f, cx, cy + 3.f);
    g.setColour (kYellow);
    g.fillPath (arrow);
}

void TwixiLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (7, 1, box.getWidth() - 24, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void TwixiLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    if (b.getProperties().contains ("pill"))
    {
        auto r = b.getLocalBounds().toFloat().reduced (1.5f);
        g.setColour (b.getToggleState() ? kLime : juce::Colour (0xff2a2a8c));
        g.fillRoundedRectangle (r, 9.f);
        g.setColour (kInk);
        g.drawRoundedRectangle (r, 9.f, 2.5f);
        g.setColour (b.getToggleState() ? kInk : juce::Colours::white);
        g.setFont (juce::Font (11.f, juce::Font::bold));
        g.drawText (b.getButtonText(), r, juce::Justification::centred);
        return;
    }
    auto r = b.getLocalBounds().toFloat().reduced (2.f);
    const auto col = b.findColour (juce::ToggleButton::tickColourId);
    g.setColour (b.getToggleState() ? col : juce::Colour (0xff1a2a78));
    g.fillEllipse (r);
    if (b.getToggleState())
    {
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.fillEllipse (r.getX() + 4.f, r.getY() + 3.f, r.getWidth() * 0.35f, r.getHeight() * 0.25f);
    }
    g.setColour (highlighted ? juce::Colours::white : kInk);
    g.drawEllipse (r, 2.5f);
}

//==============================================================================
EqGraph::EqGraph (TwixiAudioProcessor& p)
    : proc (p), fft (TwixiAudioProcessor::fftOrder),
      window (TwixiAudioProcessor::fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    smooth.fill (-120.f);
    startTimerHz (30);
}

void EqGraph::resized()
{
    plot = getLocalBounds().toFloat().withTrimmedLeft (40.f).withTrimmedRight (26.f)
                                     .withTrimmedTop (14.f).withTrimmedBottom (26.f);
}

float EqGraph::freqToX (float f) const { return plot.getX() + plot.getWidth() * std::log (f / 20.f) / std::log (1000.f); }
float EqGraph::xToFreq (float x) const { return 20.f * std::pow (1000.f, (x - plot.getX()) / plot.getWidth()); }
float EqGraph::gainToY (float db, float range) const { return plot.getCentreY() - db / range * plot.getHeight() * 0.5f; }
float EqGraph::yToGain (float y, float range) const { return (plot.getCentreY() - y) / (plot.getHeight() * 0.5f) * range; }

juce::Point<float> EqGraph::nodePos (const BandSettings& s, float range) const
{
    return { freqToX (s.freq), gainToY (s.hasGain() ? s.gain : 0.f, range) };
}

int EqGraph::hitTest (juce::Point<float> p) const
{
    const float range = proc.getRangeDb();
    int best = -1;
    float bestD = 15.f;
    for (int b = 0; b < kNumBands; ++b)
    {
        const auto s = proc.getBandSettings (b);
        if (! s.on) continue;
        const float d = p.getDistanceFrom (nodePos (s, range));
        if (d <= bestD) { bestD = d; best = b; }
    }
    return best;
}

void EqGraph::setParam (const juce::String& id, float v)
{
    if (auto* p = proc.apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void EqGraph::setParamWithGesture (const juce::String& id, float v)
{
    if (auto* p = proc.apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
}

//------------------------------------------------------------------------------
void EqGraph::timerCallback()
{
    if (proc.isAnalyzerOn())
    {
        constexpr int N = TwixiAudioProcessor::fftSize;
        proc.copyAnalyzerBlock (fftData.data());
        window.multiplyWithWindowingTable (fftData.data(), (size_t) N);
        std::fill (fftData.begin() + N, fftData.end(), 0.f);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        for (int i = 0; i < N / 2; ++i)
        {
            const float db = juce::Decibels::gainToDecibels (fftData[(size_t) i] / ((float) N * 0.25f), -120.f);
            float& s = smooth[(size_t) i];
            s += (db - s) * (db > s ? 0.6f : 0.12f);
        }
    }
    repaint();
}

void EqGraph::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float range = proc.getRangeDb();
    double sr = proc.getSampleRate();
    if (sr <= 0.0) sr = 44100.0;

    // panel
    g.setColour (juce::Colour (0xe0071545));
    g.fillRoundedRectangle (bounds.reduced (2.f), 14.f);
    g.setColour (kInk);
    g.drawRoundedRectangle (bounds.reduced (2.f), 14.f, 3.f);

    // rejilla de frecuencia
    g.setFont (juce::Font (11.f, juce::Font::bold));
    const float freqs[] = { 20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f };
    for (float f : freqs)
    {
        const float x = freqToX (f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        const juce::String t = f >= 1000.f ? juce::String ((int) (f / 1000.f)) + "k" : juce::String ((int) f);
        g.drawText (t, juce::Rectangle<float> (x - 20.f, plot.getBottom() + 5.f, 40.f, 14.f), juce::Justification::centred);
    }

    // rejilla de dB
    const float step = range <= 6.f ? 3.f : 6.f;
    for (float db = -range; db <= range + 0.01f; db += step)
    {
        const float y = gainToY (db, range);
        g.setColour (juce::Colours::white.withAlpha (std::abs (db) < 0.01f ? 0.35f : 0.10f));
        g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawText ((db > 0.01f ? "+" : "") + juce::String ((int) db),
                    juce::Rectangle<float> (4.f, y - 7.f, 32.f, 14.f), juce::Justification::centredRight);
    }

    g.saveState();
    g.reduceClipRegion (plot.expanded (2.f, 4.f).toNearestInt());

    // analizador de espectro
    if (proc.isAnalyzerOn())
    {
        constexpr int N = TwixiAudioProcessor::fftSize;
        constexpr int half = N / 2;
        juce::Path a;
        a.startNewSubPath (plot.getX(), plot.getBottom());
        for (float x = plot.getX(); x <= plot.getRight(); x += 2.f)
        {
            const float f = xToFreq (x);
            const int lo = juce::jmax (1, (int) std::floor (xToFreq (x - 1.f) * N / (float) sr));
            const int hi = juce::jmin (half - 1, (int) std::ceil (xToFreq (x + 1.f) * N / (float) sr));
            float v;
            if (hi - lo <= 1)
            {
                const float pos = f * N / (float) sr;
                const int i0 = juce::jlimit (1, half - 2, (int) std::floor (pos));
                const float fr = juce::jlimit (0.f, 1.f, pos - (float) i0);
                v = smooth[(size_t) i0] * (1.f - fr) + smooth[(size_t) (i0 + 1)] * fr;
            }
            else
            {
                v = -120.f;
                for (int i = lo; i <= hi; ++i) v = std::max (v, smooth[(size_t) i]);
            }
            v += 4.5f * std::log2 (f / 1000.f);                 // compensacion de pendiente
            const float norm = juce::jlimit (0.f, 1.f, (v + 84.f) / 78.f);
            a.lineTo (x, plot.getBottom() - norm * plot.getHeight());
        }
        a.lineTo (plot.getRight(), plot.getBottom());
        a.closeSubPath();
        g.setGradientFill (juce::ColourGradient (kLime.withAlpha (0.40f), 0.f, plot.getY(),
                                                 kLime.withAlpha (0.03f), 0.f, plot.getBottom(), false));
        g.fillPath (a);
        g.setColour (kLime.withAlpha (0.75f));
        g.strokePath (a, juce::PathStrokeType (1.5f));
    }

    // curvas de respuesta
    BandSettings s[kNumBands];
    BiquadCoeffs co[kNumBands][kMaxStages];
    int ns[kNumBands];
    for (int b = 0; b < kNumBands; ++b)
    {
        s[b] = proc.getBandSettings (b);
        ns[b] = s[b].on ? designBand (s[b], sr, co[b]) : 0;
    }

    const int numPts = (int) (plot.getWidth() / 2.f) + 1;
    std::vector<float> total ((size_t) numPts, 0.f);
    const float y0 = gainToY (0.f, range);
    const float lim = range * 1.6f;

    for (int b = 0; b < kNumBands; ++b)
    {
        if (ns[b] == 0) continue;
        juce::Path fill;
        fill.startNewSubPath (plot.getX(), y0);
        for (int p = 0; p < numPts; ++p)
        {
            const float x = plot.getX() + (float) p * 2.f;
            double db = 0.0;
            for (int k = 0; k < ns[b]; ++k) db += magnitudeDb (co[b][k], xToFreq (x), sr);
            total[(size_t) p] += (float) db;
            fill.lineTo (x, gainToY (juce::jlimit (-lim, lim, (float) db), range));
        }
        fill.lineTo (plot.getX() + (float) (numPts - 1) * 2.f, y0);
        fill.closeSubPath();
        g.setColour (bandColour (b).withAlpha (0.16f));
        g.fillPath (fill);
    }

    juce::Path curve;
    for (int p = 0; p < numPts; ++p)
    {
        const float x = plot.getX() + (float) p * 2.f;
        const float y = gainToY (juce::jlimit (-lim, lim, total[(size_t) p]), range);
        if (p == 0) curve.startNewSubPath (x, y); else curve.lineTo (x, y);
    }
    g.setColour (kInk);
    g.strokePath (curve, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (kYellow);
    g.strokePath (curve, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.restoreState();

    // nodos
    g.setFont (juce::Font (11.f, juce::Font::bold));
    for (int b = 0; b < kNumBands; ++b)
    {
        if (! s[b].on) continue;
        const auto pt = nodePos (s[b], range);
        const float r = (b == hoverBand || b == dragBand) ? 12.f : 10.f;
        g.setColour (kInk);
        g.fillEllipse (pt.x - r - 2.f, pt.y - r - 2.f, 2.f * r + 4.f, 2.f * r + 4.f);
        g.setColour (bandColour (b));
        g.fillEllipse (pt.x - r, pt.y - r, 2.f * r, 2.f * r);
        g.setColour (kInk);
        g.drawText (juce::String (b + 1), juce::Rectangle<float> (pt.x - r, pt.y - r, 2.f * r, 2.f * r), juce::Justification::centred);
    }
}

//------------------------------------------------------------------------------
void EqGraph::mouseMove (const juce::MouseEvent& e)
{
    const int h = hitTest (e.position);
    if (h != hoverBand) { hoverBand = h; repaint(); }
}

void EqGraph::mouseDown (const juce::MouseEvent& e)
{
    dragBand = hitTest (e.position);
    if (dragBand >= 0)
        for (auto id : { "freq", "gain" })
            if (auto* p = proc.apvts.getParameter (bandParamId (dragBand, id))) p->beginChangeGesture();
}

void EqGraph::mouseDrag (const juce::MouseEvent& e)
{
    if (dragBand < 0) return;
    const float range = proc.getRangeDb();
    const auto s = proc.getBandSettings (dragBand);
    const float x = juce::jlimit (plot.getX(), plot.getRight(), e.position.x);
    const float y = juce::jlimit (plot.getY(), plot.getBottom(), e.position.y);
    setParam (bandParamId (dragBand, "freq"), xToFreq (x));
    if (s.hasGain())
        setParam (bandParamId (dragBand, "gain"), juce::jlimit (-24.f, 24.f, yToGain (y, range)));
}

void EqGraph::mouseUp (const juce::MouseEvent&)
{
    if (dragBand >= 0)
        for (auto id : { "freq", "gain" })
            if (auto* p = proc.apvts.getParameter (bandParamId (dragBand, id))) p->endChangeGesture();
    dragBand = -1;
    repaint();
}

void EqGraph::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int hit = hitTest (e.position);
    if (hit >= 0)
    {
        setParamWithGesture (bandParamId (hit, "on"), 0.f);       // doble clic en nodo: apagar banda
        return;
    }
    for (int b = 0; b < kNumBands; ++b)                            // doble clic vacio: activar banda libre
    {
        auto s = proc.getBandSettings (b);
        if (s.on) continue;
        const float range = proc.getRangeDb();
        setParamWithGesture (bandParamId (b, "freq"), xToFreq (juce::jlimit (plot.getX(), plot.getRight(), e.position.x)));
        if (s.hasGain())
            setParamWithGesture (bandParamId (b, "gain"), juce::jlimit (-24.f, 24.f, yToGain (e.position.y, range)));
        setParamWithGesture (bandParamId (b, "on"), 1.f);
        break;
    }
}

void EqGraph::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    const int hit = hitTest (e.position);
    if (hit < 0) return;
    const auto s = proc.getBandSettings (hit);
    if (s.isCut()) return;
    const float dir = w.isReversed ? -1.f : 1.f;
    setParamWithGesture (bandParamId (hit, "q"), juce::jlimit (0.1f, 18.f, s.q * std::exp (w.deltaY * dir * 1.5f)));
}

//==============================================================================
TwixiEditor::TwixiEditor (TwixiAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), graph (p)
{
    setLookAndFeel (&laf);
    setSize (960, 660);
    setResizable (false, false);

    addAndMakeVisible (graph);

    for (int i = 0; i < kNumBands; ++i)
    {
        auto& u = bands[(size_t) i];
        const auto col = bandColour (i);

        u.on.setButtonText ({});
        u.on.setColour (juce::ToggleButton::tickColourId, col);
        u.type.addItemList (choicesOf (bandParamId (i, "type")), 1);
        u.slope.addItemList (choicesOf (bandParamId (i, "slope")), 1);
        u.channel.addItemList (choicesOf (bandParamId (i, "ch")), 1);
        initKnob (u.freq, col);
        initKnob (u.gain, col);
        initKnob (u.q, col);

        for (juce::Component* c : std::initializer_list<juce::Component*> { &u.on, &u.type, &u.slope, &u.channel, &u.freq, &u.gain, &u.q })
            addAndMakeVisible (c);

        u.onA      = std::make_unique<ButtonAtt> (proc.apvts, bandParamId (i, "on"), u.on);
        u.typeA    = std::make_unique<ComboAtt>  (proc.apvts, bandParamId (i, "type"), u.type);
        u.slopeA   = std::make_unique<ComboAtt>  (proc.apvts, bandParamId (i, "slope"), u.slope);
        u.channelA = std::make_unique<ComboAtt>  (proc.apvts, bandParamId (i, "ch"), u.channel);
        u.freqA    = std::make_unique<SliderAtt> (proc.apvts, bandParamId (i, "freq"), u.freq);
        u.gainA    = std::make_unique<SliderAtt> (proc.apvts, bandParamId (i, "gain"), u.gain);
        u.qA       = std::make_unique<SliderAtt> (proc.apvts, bandParamId (i, "q"), u.q);
    }

    initKnob (outGain, kLime);
    outGain.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
    addAndMakeVisible (outGain);
    outGainA = std::make_unique<SliderAtt> (proc.apvts, "out_gain", outGain);

    rangeBox.addItemList (choicesOf ("range"), 1);
    addAndMakeVisible (rangeBox);
    rangeA = std::make_unique<ComboAtt> (proc.apvts, "range", rangeBox);

    analyzerBtn.getProperties().set ("pill", true);
    addAndMakeVisible (analyzerBtn);
    analyzerA = std::make_unique<ButtonAtt> (proc.apvts, "analyzer", analyzerBtn);

    resized();
    timerCallback();
    startTimerHz (15);
}

TwixiEditor::~TwixiEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void TwixiEditor::initKnob (juce::Slider& s, juce::Colour c)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 62, 14);
    s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    s.setColour (juce::Slider::rotarySliderFillColourId, c);
}

juce::StringArray TwixiEditor::choicesOf (const juce::String& id)
{
    if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        return c->choices;
    return {};
}

juce::Rectangle<int> TwixiEditor::panelBounds (int i) const
{
    return { 22 + i * (146 + 8), 414, 146, 232 };
}

void TwixiEditor::resized()
{
    graph.setBounds (20, 124, 920, 280);

    for (int i = 0; i < kNumBands; ++i)
    {
        auto& u = bands[(size_t) i];
        const auto p = panelBounds (i);
        u.on.setBounds      (p.getX() + 8,   p.getY() + 8,   22, 22);
        u.type.setBounds    (p.getX() + 8,   p.getY() + 36,  130, 24);
        u.freq.setBounds    (p.getX() + 8,   p.getY() + 78,  62, 66);
        u.gain.setBounds    (p.getX() + 76,  p.getY() + 78,  62, 66);
        u.slope.setBounds   (p.getX() + 76,  p.getY() + 100, 62, 24);
        u.q.setBounds       (p.getX() + 8,   p.getY() + 160, 62, 66);
        u.channel.setBounds (p.getX() + 76,  p.getY() + 180, 62, 24);
    }

    analyzerBtn.setBounds (782, 60, 80, 22);
    rangeBox.setBounds    (782, 90, 80, 24);
    outGain.setBounds     (874, 6, 72, 92);

    renderBackground();
}

//------------------------------------------------------------------------------
void TwixiEditor::renderBackground()
{
    const int W = getWidth(), H = getHeight();
    background = juce::Image (juce::Image::ARGB, W, H, true);
    juce::Graphics g (background);
    const float cx = 480.f;

    // cielo nocturno
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0d3fb0), 0.f, 0.f, juce::Colour (0xff08266e), 0.f, (float) H, false));
    g.fillAll();
    g.setColour (kInk);
    g.drawRect (0, 0, W, H, 4);

    // estrellas, nubes, luna
    const float stars[][3] = { { 350, 34, 9 }, { 600, 30, 10 }, { 660, 98, 6 }, { 790, 26, 8 }, { 925, 40, 8 }, { 300, 100, 6 }, { 22, 40, 7 }, { 560, 100, 5 } };
    for (auto& st : stars) drawStar (g, st[0], st[1], st[2]);
    drawCloud (g, -20.f, 84.f, 120.f);
    drawCloud (g, 690.f, 88.f, 90.f);

    auto crescent = [&] (float R, float Rc, juce::Colour c)
    {
        g.saveState();
        juce::Path cl;
        cl.setUsingNonZeroWinding (false);
        cl.addRectangle (640.f, 0.f, 160.f, 120.f);
        cl.addEllipse (732.f - Rc, 32.f - Rc, 2.f * Rc, 2.f * Rc);
        g.reduceClipRegion (cl);
        g.setColour (c);
        g.fillEllipse (716.f - R, 44.f - R, 2.f * R, 2.f * R);
        g.restoreState();
    };
    crescent (32.f, 21.f, kInk);
    crescent (28.f, 24.f, kYellow);

    // rayo tractor
    juce::Path beam;
    beam.startNewSubPath (cx - 45.f, 112.f);
    beam.lineTo (cx + 45.f, 112.f);
    beam.lineTo (cx + 560.f, (float) H);
    beam.lineTo (cx - 560.f, (float) H);
    beam.closeSubPath();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff0ff4a), 0.f, 112.f, juce::Colour (0xff6fe026), 0.f, (float) H, false));
    g.fillPath (beam);
    for (int k = -2; k <= 2; ++k)
    {
        juce::Path ray;
        ray.startNewSubPath (cx + k * 14.f, 112.f);
        ray.lineTo (cx + k * 14.f + 10.f, 112.f);
        ray.lineTo (cx + k * 200.f + 50.f, (float) H);
        ray.lineTo (cx + k * 200.f - 50.f, (float) H);
        ray.closeSubPath();
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.fillPath (ray);
    }

    drawUfo (g, cx);

    // titulo
    drawOutlinedText (g, "TWIXI-6 EQ", { 24.f, 14.f, 320.f, 52.f }, juce::Font (40.f, juce::Font::bold), kYellow, 8.f);
    drawOutlinedText (g, "6-BAND ALIEN EQUALIZER", { 26.f, 62.f, 300.f, 18.f }, juce::Font (13.f, juce::Font::bold), kLime, 5.f);
    drawOutlinedText (g, "OUTPUT", { 874.f, 100.f, 72.f, 18.f }, juce::Font (12.f, juce::Font::bold), juce::Colours::white, 5.f, juce::Justification::centred);

    // paneles de bandas
    for (int i = 0; i < kNumBands; ++i)
    {
        const auto p = panelBounds (i).toFloat();
        g.setColour (juce::Colour (0xe0071545));
        g.fillRoundedRectangle (p, 12.f);
        g.setColour (bandColour (i));
        g.fillRoundedRectangle (p.getX() + 10.f, p.getY() + 3.f, p.getWidth() - 20.f, 4.f, 2.f);
        g.setColour (kInk);
        g.drawRoundedRectangle (p, 12.f, 3.f);
        drawOutlinedText (g, "BAND " + juce::String (i + 1), { p.getX() + 38.f, p.getY() + 9.f, 100.f, 20.f },
                          juce::Font (14.f, juce::Font::bold), bandColour (i), 4.f);
    }
}

void TwixiEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (background, 0, 0);

    g.setFont (juce::Font (10.f, juce::Font::bold));
    g.setColour (juce::Colours::white.withAlpha (0.8f));
    for (int i = 0; i < kNumBands; ++i)
    {
        const auto s = proc.getBandSettings (i);
        const auto p = panelBounds (i);
        auto cap = [&] (const juce::String& t, int x, int y)
        {
            g.drawText (t, juce::Rectangle<int> (p.getX() + x, p.getY() + y, 62, 14), juce::Justification::centred);
        };
        cap ("FREQ", 8, 64);
        if (s.hasGain()) cap ("GAIN", 76, 64);
        if (s.isCut())   cap ("SLOPE", 76, 84);
        if (! s.isCut()) cap ("Q", 8, 146);
        cap ("CHANNEL", 76, 164);
    }
}

void TwixiEditor::timerCallback()
{
    bool changed = false;
    for (int i = 0; i < kNumBands; ++i)
    {
        auto& u = bands[(size_t) i];
        const auto s = proc.getBandSettings (i);
        if (s.type == u.lastType && s.on == u.lastOn) continue;

        u.lastType = s.type;
        u.lastOn = s.on;
        u.gain.setVisible (s.hasGain());
        u.slope.setVisible (s.isCut());
        u.q.setVisible (! s.isCut());

        const float a = s.on ? 1.f : 0.45f;
        for (juce::Component* c : std::initializer_list<juce::Component*> { &u.type, &u.slope, &u.channel, &u.freq, &u.gain, &u.q })
            c->setAlpha (a);
        changed = true;
    }
    if (changed) repaint();
}
