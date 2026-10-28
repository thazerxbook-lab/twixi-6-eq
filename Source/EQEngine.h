#pragma once
#include <algorithm>
#include <cmath>
#include <complex>

// Twixi-6 EQ - motor DSP (sin dependencias de JUCE, para poder probarlo solo)
namespace twixi
{
constexpr int kNumBands  = 6;
constexpr int kMaxStages = 4;      // 48 dB/oct = 4 biquads
constexpr double kPi     = 3.14159265358979323846;

enum BandType { Bell = 0, LowShelf, HighShelf, LowCut, HighCut, Notch };
enum ChannelMode { Stereo = 0, Mid, Side };

struct BandSettings
{
    bool  on      = false;
    int   type    = Bell;
    float freq    = 1000.f;
    float gain    = 0.f;
    float q       = 0.71f;
    int   slope   = 1;      // 0=12, 1=24, 2=48 dB/oct
    int   channel = Stereo;

    bool hasGain() const { return type == Bell || type == LowShelf || type == HighShelf; }
    bool isCut()   const { return type == LowCut || type == HighCut; }

    bool operator== (const BandSettings& o) const
    {
        return on == o.on && type == o.type && freq == o.freq && gain == o.gain
            && q == o.q && slope == o.slope && channel == o.channel;
    }
};

struct BiquadCoeffs { double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0; };

// Formulas del "Audio EQ Cookbook" (RBJ)
inline BiquadCoeffs makeBiquad (int type, double sr, double f, double gainDb, double q)
{
    const double w0 = 2.0 * kPi * f / sr;
    const double cw = std::cos (w0), sw = std::sin (w0);
    const double alpha = sw / (2.0 * q);
    const double A = std::pow (10.0, gainDb / 40.0);
    double b0 = 1, b1 = 0, b2 = 0, a0 = 1, a1 = 0, a2 = 0;

    switch (type)
    {
        case Bell:
            b0 = 1 + alpha * A;  b1 = -2 * cw;  b2 = 1 - alpha * A;
            a0 = 1 + alpha / A;  a1 = -2 * cw;  a2 = 1 - alpha / A;
            break;
        case LowShelf:
        {
            const double beta = 2.0 * std::sqrt (A) * alpha;
            b0 =      A * ((A + 1) - (A - 1) * cw + beta);
            b1 =  2 * A * ((A - 1) - (A + 1) * cw);
            b2 =      A * ((A + 1) - (A - 1) * cw - beta);
            a0 =           (A + 1) + (A - 1) * cw + beta;
            a1 =     -2 * ((A - 1) + (A + 1) * cw);
            a2 =           (A + 1) + (A - 1) * cw - beta;
            break;
        }
        case HighShelf:
        {
            const double beta = 2.0 * std::sqrt (A) * alpha;
            b0 =      A * ((A + 1) + (A - 1) * cw + beta);
            b1 = -2 * A * ((A - 1) + (A + 1) * cw);
            b2 =      A * ((A + 1) + (A - 1) * cw - beta);
            a0 =           (A + 1) - (A - 1) * cw + beta;
            a1 =      2 * ((A - 1) - (A + 1) * cw);
            a2 =           (A + 1) - (A - 1) * cw - beta;
            break;
        }
        case LowCut:   // high-pass
            b0 = (1 + cw) / 2;  b1 = -(1 + cw);  b2 = (1 + cw) / 2;
            a0 = 1 + alpha;     a1 = -2 * cw;    a2 = 1 - alpha;
            break;
        case HighCut:  // low-pass
            b0 = (1 - cw) / 2;  b1 = 1 - cw;     b2 = (1 - cw) / 2;
            a0 = 1 + alpha;     a1 = -2 * cw;    a2 = 1 - alpha;
            break;
        case Notch:
            b0 = 1;             b1 = -2 * cw;    b2 = 1;
            a0 = 1 + alpha;     a1 = -2 * cw;    a2 = 1 - alpha;
            break;
        default: break;
    }
    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

// Rellena 'out' con las etapas biquad de la banda y devuelve cuantas son.
inline int designBand (const BandSettings& s, double sr, BiquadCoeffs* out)
{
    const double f = std::clamp ((double) s.freq, 10.0, sr * 0.49);
    const double q = std::max (0.1, (double) s.q);

    if (s.isCut())
    {
        // Butterworth en cascada de biquads: 12 / 24 / 48 dB por octava
        const int order = 2 << std::clamp (s.slope, 0, 2);
        const int n = order / 2;
        for (int k = 1; k <= n; ++k)
        {
            const double Q = 1.0 / (2.0 * std::sin ((2 * k - 1) * kPi / (2.0 * order)));
            out[k - 1] = makeBiquad (s.type, sr, f, 0.0, Q);
        }
        return n;
    }

    out[0] = makeBiquad (s.type, sr, f, s.gain, q);
    return 1;
}

inline double magnitudeDb (const BiquadCoeffs& c, double freq, double sr)
{
    const double w = 2.0 * kPi * freq / sr;
    const std::complex<double> z1 = std::polar (1.0, -w);
    const std::complex<double> z2 = z1 * z1;
    const std::complex<double> num = c.b0 + c.b1 * z1 + c.b2 * z2;
    const std::complex<double> den = 1.0 + c.a1 * z1 + c.a2 * z2;
    return 20.0 * std::log10 (std::max (std::abs (num) / std::max (std::abs (den), 1e-12), 1e-9));
}
} // namespace twixi
