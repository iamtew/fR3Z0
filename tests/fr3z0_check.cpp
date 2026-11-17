#include "dsp/Units.h"
#include "dsp/Bank.h"
#include "dsp/Presets.h"

#include <cstdio>
#include <cmath>
#include <cstdlib>

static int fails = 0;

static void expect (bool ok, const char* msg)
{
    if (! ok)
    {
        std::printf ("FAIL %s\n", msg);
        ++fails;
    }
}

static void expectNear (float a, float b, float eps, const char* msg)
{
    if (std::fabs (a - b) > eps)
    {
        std::printf ("FAIL %s got %f want %f\n", msg, (double) a, (double) b);
        ++fails;
    }
}

int main()
{
    using namespace fr3z0;

    expectNear (midiToHz (69.f), 440.f, 0.01f, "A4");
    expectNear (intervalToHz (440.f, 12.f, 0.f), 880.f, 0.02f, "octave");
    expectNear (intervalToHz (100.f, 0.f, 100.f), 100.f * std::pow (2.f, 1.f / 12.f), 0.05f, "cents");

    expect (clampHz (1.f) == kMinHz, "clamp low");
    expect (clampHz (20000.f) == kMaxHz, "clamp high");

    expectNear (snapMidi (61.f, (int) Scale::Major), 60.f, 0.01f, "C# to C major");
    expectNear (snapMidi (61.f, (int) Scale::Minor), 60.f, 0.01f, "C# snaps to C (tie)");
    expectNear (snapMidi (61.f, (int) Scale::Chromatic), 61.f, 0.01f, "chromatic passthrough");

    expectNear (harmonicMul (0), 1.f, 0.001f, "h1");
    expectNear (harmonicMul (3), 4.f, 0.001f, "h4");

    expect (decayToDamp (1.f) < decayToDamp (0.f), "longer decay, less damp");
    expect (intensityToQ (1.f, 1.f) > intensityToQ (0.f, 1.f), "intensity raises Q");

    expectNear (lfoWave (0, 0.f), 0.f, 0.001f, "sine 0");
    expectNear (lfoWave (0, 0.25f), 1.f, 0.001f, "sine peak");
    expectNear (lfoWave (3, 0.1f), 1.f, 0.001f, "square high");
    expectNear (lfoWave (3, 0.6f), -1.f, 0.001f, "square low");
    expectNear (applyUni (1.f, true), 1.f, 0.001f, "uni high");
    expectNear (applyUni (-1.f, true), 0.f, 0.001f, "uni low");

    LfoState s;
    float v = s.tick (0, 0.25f, false);
    expect (std::fabs (v) <= 1.001f, "lfo bounds");

    expect (kPresetCount == 10, "preset count");
    expect (kPresets[1].v[0].on > 0.5f, "minor root on");
    expect (kPresets[1].v[1].semi == 3.f, "minor third");
    expect (kPresets[2].v[1].semi == 4.f, "major third");

    Bank bank;
    bank.prepare (48000.0);
    BlockIn in;
    in.v[0].on = true;
    in.v[0].gain = 1.f;
    in.mix = 1.f;
    in.intensity = 0.8f;
    in.decay = 0.7f;
    float L[64] {}, R[64] {};
    L[0] = R[0] = 1.f;
    bank.process (L, R, 64, in);
    float energy = 0.f;
    for (int i = 0; i < 64; ++i)
        energy += std::fabs (L[i]);
    expect (energy > 0.1f, "impulse rings");
    expect (bank.lastHz[0] > 20.f, "voice hz");

    bank.reset();
    float sum = 0.f;
    for (int i = 0; i < 8; ++i)
        sum += std::fabs (bank.voices[i].l.ic1eq) + std::fabs (bank.voices[i].l.ic2eq);
    expect (sum == 0.f, "kill/reset zeros state");

    if (fails)
    {
        std::printf ("%d failed\n", fails);
        return 1;
    }
    std::printf ("ok\n");
    return 0;
}
