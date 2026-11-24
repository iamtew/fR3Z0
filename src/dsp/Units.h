#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>

namespace fr3z0
{

constexpr int kVoices = 8;
constexpr int kLfos = 2;
constexpr int kFftOrder = 10;
constexpr int kFftSize = 1 << kFftOrder;
constexpr int kFftBins = kFftSize / 2;

inline float fftBin (float hz, float sampleRate)
{
    const float sr = sampleRate > 1.f ? sampleRate : 44100.f;
    return hz * (float) kFftSize / sr;
}

// mag is |bin| / N.  Silence → 0, 0 dBFS → 1.
inline float fftMagToUnit (float mag)
{
    const float db = 20.f * std::log10 (std::max (mag, 1.0e-12f));
    return std::max (0.f, std::min (1.f, (db + 90.f) / 90.f));
}
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr float kMinHz = 20.f;
constexpr float kMaxHz = 12000.f;

inline float midiToHz (float midi)
{
    return 440.f * std::pow (2.f, (midi - 69.f) / 12.f);
}

inline float hzToMidi (float hz)
{
    return 69.f + 12.f * std::log2 (std::max (1.f, hz) / 440.f);
}

inline float wrap01 (float x)
{
    x -= std::floor (x);
    return x;
}

inline float smoothToward (float& z, float target, float a, bool snap)
{
    if (snap)
        z = target;
    else
        z += a * (target - z);
    return z;
}

inline float intervalToHz (float rootHz, float semitones, float cents)
{
    return rootHz * std::pow (2.f, (semitones + cents * 0.01f) / 12.f);
}

inline float clampHz (float hz)
{
    return std::max (kMinHz, std::min (kMaxHz, hz));
}

inline float onePoleCoeff (float hz, float sampleRate)
{
    const float f = std::max (1.f, std::min (hz, sampleRate * 0.45f));
    return 1.f - std::exp (-kTwoPi * f / sampleRate);
}

inline float softclip (float x)
{
    if (x > 4.f) return 1.f;
    if (x < -4.f) return -1.f;
    return std::tanh (x);
}

// Decay 0..1 → extra SVF damping. 1 = longest ring.
inline float decayToDamp (float decay)
{
    const float d = std::max (0.f, std::min (1.f, decay));
    return 0.02f + (1.f - d) * 0.35f;
}

inline float intensityToQ (float intensity, float voiceQ)
{
    const float i = std::max (0.f, std::min (1.f, intensity));
    const float vq = std::max (0.2f, std::min (2.f, voiceQ));
    return (2.f + i * 36.f) * vq;
}

enum class Scale : int { Chromatic = 0, Major, Minor, Harmonic };

inline constexpr int kMajor[] = { 0, 2, 4, 5, 7, 9, 11 };
inline constexpr int kMinor[] = { 0, 2, 3, 5, 7, 8, 10 };

inline int nearestPc (int pc, const int* degrees, int n)
{
    pc = ((pc % 12) + 12) % 12;
    int best = degrees[0];
    int bestD = 12;
    for (int i = 0; i < n; ++i)
    {
        int d = std::abs (pc - degrees[i]);
        d = std::min (d, 12 - d);
        if (d < bestD) { bestD = d; best = degrees[i]; }
    }
    return best;
}

// Snap a MIDI note (float) onto the scale. Chromatic and Harmonic leave it.
inline float snapMidi (float midi, int scale)
{
    if (scale == (int) Scale::Chromatic || scale == (int) Scale::Harmonic)
        return midi;
    const int* deg = scale == (int) Scale::Minor ? kMinor : kMajor;
    const int oct = (int) std::floor (midi / 12.f);
    float best = midi;
    float bestD = 99.f;
    for (int o = oct - 1; o <= oct + 1; ++o)
        for (int i = 0; i < 7; ++i)
        {
            const float cand = (float) (o * 12 + deg[i]);
            const float d = std::abs (midi - cand);
            if (d < bestD) { bestD = d; best = cand; }
        }
    return best;
}

inline float harmonicMul (int voiceIndex)
{
    return (float) (voiceIndex + 1);
}

// Inverse of linked/unlinked voiceHz. Harmonic+link cannot set Hz via interval.
struct VoiceTune
{
    float semi = 0.f;
    float cents = 0.f;
    bool writeInterval = true;
};

inline VoiceTune intervalFromHz (float hz, float rootMidi, bool link, int scale, bool snap)
{
    VoiceTune t;
    if (scale == (int) Scale::Harmonic && link)
    {
        t.writeInterval = false;
        return t;
    }
    float midi = hzToMidi (clampHz (hz));
    if (snap)
        midi = snapMidi (midi, scale);
    const float base = link ? rootMidi : 60.f;
    float delta = midi - base;
    if (snap)
    {
        t.semi = std::max (-24.f, std::min (24.f, delta));
        t.cents = 0.f;
        return t;
    }
    float semi = std::floor (delta);
    float cents = (delta - semi) * 100.f;
    if (cents > 50.f)
    {
        semi += 1.f;
        cents -= 100.f;
    }
    t.semi = std::max (-24.f, std::min (24.f, semi));
    t.cents = std::max (-50.f, std::min (50.f, cents));
    return t;
}

inline float lfoWave (int shape, float phase)
{
    // phase 0..1. Bipolar -1..1.
    switch (shape)
    {
        case 1: // triangle
            return 4.f * std::abs (phase - 0.5f) - 1.f;
        case 2: // saw
            return phase * 2.f - 1.f;
        case 3: // square
            return phase < 0.5f ? 1.f : -1.f;
        case 4: // sample & hold uses stored hold; wave is unused
            return 0.f;
        default: // sine
            return std::sin (phase * kTwoPi);
    }
}

inline float applyUni (float bipolar, bool unipolar)
{
    return unipolar ? bipolar * 0.5f + 0.5f : bipolar;
}

// Sync note: 0=1/1 .. 5=1/16T. Cycles per beat.
inline float syncCyclesPerBeat (int div)
{
    switch (div)
    {
        case 1: return 0.5f;
        case 2: return 1.f;
        case 3: return 2.f;
        case 4: return 3.f;   // 1/8T
        case 5: return 6.f;   // 1/16T
        default: return 0.25f; // 1/1 over 4 beats → 0.25 cycle/beat? 1/1 = 1 bar = 4 beats = 0.25
    }
}

// TPT SVF, bandpass out. Cytomic form.
struct Svf
{
    float ic1eq = 0.f, ic2eq = 0.f;

    void reset() { ic1eq = ic2eq = 0.f; }

    float band (float v0, float g, float k)
    {
        const float a1 = 1.f / (1.f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;
        const float v3 = v0 - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a3 * v3;
        ic1eq = 2.f * v1 - ic1eq;
        ic2eq = 2.f * v2 - ic2eq;
        return v1;
    }
};

struct OnePole
{
    float z = 0.f;
    void reset() { z = 0.f; }
    float lp (float x, float a)
    {
        z += a * (x - z);
        return z;
    }
    float hp (float x, float a) { return x - lp (x, a); }
};

struct Pink
{
    float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;
    void reset() { b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0; }
    float process (float white)
    {
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        const float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        b6 = white * 0.115926f;
        return pink * 0.11f;
    }
};

struct Lcg
{
    std::uint32_t s = 1u;
    float next()
    {
        s = s * 1664525u + 1013904223u;
        return (float) ((s >> 8) & 0xffffff) / 16777216.f * 2.f - 1.f;
    }
};

struct LfoState
{
    float phase = 0.f;
    float hold = 0.f;
    Lcg rng;

    float tick (int shape, float inc, bool unipolar, float phaseOff = 0.f)
    {
        const float prevOff = wrap01 (phase + phaseOff);
        phase = wrap01 (phase + inc);
        const float off = wrap01 (phase + phaseOff);
        if (shape == 4 && off < prevOff)
            hold = rng.next();
        const float bi = (shape == 4) ? hold : lfoWave (shape, off);
        return applyUni (bi, unipolar);
    }
};

} // namespace fr3z0
