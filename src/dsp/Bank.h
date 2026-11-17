#pragma once

#include "Units.h"

#include <algorithm>
#include <cmath>

namespace fr3z0
{

struct VoiceIn
{
    bool on = false;
    bool link = true;
    float semi = 0, cents = 0, gain = 0.7f, pan = 0, decay = 0.5f, q = 1.f;
};

struct LfoIn
{
    int shape = 0, div = 2;
    bool sync = false, uni = false;
    float rate = 0.5f, depth = 0, phaseOff = 0;
    float aRoot = 0, aDecay = 0, aInt = 0, aWidth = 0, aMix = 0;
    float aVFreq = 0, aVGain = 0, aVPan = 0, aVDecay = 0;
};

struct BlockIn
{
    float rootMidi = 60, decay = 0.5f, intensity = 0.4f, mix = 0.5f, outGain = 1.f;
    float width = 0.5f, detune = 0, hpHz = 20, lpHz = 16000, noise = 0, color = 0.7f;
    bool freeze = false;
    int scale = 0;
    bool snap = false;
    float bpm = 120.f;
    VoiceIn v[kVoices];
    LfoIn lfo[kLfos];
};

struct VoiceState
{
    Svf l, r;
    float energy = 0.f;
    void reset() { l.reset(); r.reset(); energy = 0.f; }
};

struct Bank
{
    VoiceState voices[kVoices];
    OnePole hpL, hpR, lpL, lpR, colorL, colorR;
    Pink pinkL, pinkR;
    Lcg white;
    LfoState lfo[kLfos];
    float sr = 44100.f;
    float lastHz[kVoices] {};
    float lastQ[kVoices] {};
    float lastPan[kVoices] {};
    float lastGain[kVoices] {};
    float lastEnergy[kVoices] {};
    float lastLfo[kLfos] {};
    float smMix = 0.5f, smOut = 1.f, smNoise = 0.f;
    float smHpA = 0.f, smLpA = 0.f, smColorA = 0.f;
    float smGL[kVoices] {}, smGR[kVoices] {}, smK[kVoices] {}, smGain[kVoices] {}, smPan[kVoices] {};
    bool smoothSnap = true;

    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        reset();
        lfo[0].rng.s = 1u;
        lfo[1].rng.s = 2u;
    }

    void reset()
    {
        for (auto& v : voices)
            v.reset();
        hpL.reset(); hpR.reset(); lpL.reset(); lpR.reset();
        colorL.reset(); colorR.reset();
        pinkL.reset(); pinkR.reset();
        smoothSnap = true;
    }

    static float gFromHz (float hz, float sampleRate)
    {
        const float ny = sampleRate * 0.49f;
        const float f = std::max (kMinHz, std::min (hz, ny));
        return std::tan (kPi * f / sampleRate);
    }

    float voiceHz (const BlockIn& p, int i, float rootMidi, float rootHz, float modVFreq) const
    {
        const auto& v = p.v[i];
        if (p.scale == (int) Scale::Harmonic && v.link)
            return clampHz (rootHz * harmonicMul (i));
        float midi = (v.link ? rootMidi : 60.f) + v.semi + v.cents * 0.01f + modVFreq * 7.f;
        if (p.snap)
            midi = snapMidi (midi, p.scale);
        return clampHz (midiToHz (midi));
    }

    void process (float* left, float* right, int n, const BlockIn& p)
    {
        float modRoot = 0, modDecay = 0, modInt = 0, modWidth = 0, modMix = 0;
        float modVFreq = 0, modVGain = 0, modVPan = 0, modVDecay = 0;

        const float invSr = 1.f / sr;
        for (int li = 0; li < kLfos; ++li)
        {
            const auto& L = p.lfo[li];
            float hz = L.rate;
            if (L.sync)
                hz = (p.bpm / 60.f) * syncCyclesPerBeat (L.div);
            const float val = lfo[li].tick (L.shape, std::max (0.f, hz) * invSr * (float) std::max (1, n),
                                           L.uni, L.phaseOff);
            lastLfo[li] = val;
            const float d = L.depth * val;
            modRoot   += d * L.aRoot;
            modDecay  += d * L.aDecay;
            modInt    += d * L.aInt;
            modWidth  += d * L.aWidth;
            modMix    += d * L.aMix;
            modVFreq  += d * L.aVFreq;
            modVGain  += d * L.aVGain;
            modVPan   += d * L.aVPan;
            modVDecay += d * L.aVDecay;
        }

        const float rootMidi = std::clamp (p.rootMidi + modRoot * 12.f, 12.f, 108.f);
        const float decayG = std::clamp (p.decay + modDecay, 0.f, 1.f);
        const float intensity = std::clamp (p.intensity + modInt, 0.f, 1.f);
        const float width = std::clamp (p.width + modWidth, 0.f, 1.f);
        const float mix = std::clamp (p.mix + modMix, 0.f, 1.f);
        const float rootHz = midiToHz (rootMidi);
        const float hpA = onePoleCoeff (p.hpHz, sr);
        const float lpA = onePoleCoeff (p.lpHz, sr);
        const float colorA = onePoleCoeff (400.f + p.color * 14000.f, sr);
        const float detRatio = std::pow (2.f, (p.detune * 0.01f) / 12.f);

        float gL[kVoices], gR[kVoices], k[kVoices], gain[kVoices];
        bool on[kVoices];
        for (int i = 0; i < kVoices; ++i)
        {
            const auto& v = p.v[i];
            on[i] = v.on;
            if (! v.on)
            {
                lastHz[i] = lastGain[i] = lastEnergy[i] = 0;
                lastQ[i] = 1;
                lastPan[i] = 0;
                voices[i].energy *= 0.9f;
                lastEnergy[i] = voices[i].energy;
                gL[i] = gR[i] = 0;
                k[i] = 1;
                gain[i] = 0;
                continue;
            }
            const float hz = voiceHz (p, i, rootMidi, rootHz, modVFreq);
            const float vDecay = std::clamp (v.decay + modVDecay, 0.f, 1.f);
            const float q = std::max (0.7f, intensityToQ (intensity, v.q)
                                            / (1.f + decayToDamp (decayG * 0.5f + vDecay * 0.5f) * 6.f));
            const float pan = std::clamp (v.pan * (0.15f + 0.85f * width) + modVPan, -1.f, 1.f);
            lastHz[i] = hz;
            lastQ[i] = q;
            lastPan[i] = pan;
            lastGain[i] = std::max (0.f, v.gain + modVGain);
            gL[i] = gFromHz (hz / detRatio, sr);
            gR[i] = gFromHz (hz * detRatio, sr);
            k[i] = 1.f / q;
            gain[i] = lastGain[i];
        }

        const float smA = onePoleCoeff (20.f, sr);
        const bool snap = smoothSnap;
        smoothSnap = false;

        for (int s = 0; s < n; ++s)
        {
            const float mixS = smoothToward (smMix, mix, smA, snap && s == 0);
            const float outS = smoothToward (smOut, p.outGain, smA, snap && s == 0);
            const float noiseS = smoothToward (smNoise, p.noise, smA, snap && s == 0);
            const float hpS = smoothToward (smHpA, hpA, smA, snap && s == 0);
            const float lpS = smoothToward (smLpA, lpA, smA, snap && s == 0);
            const float colorS = smoothToward (smColorA, colorA, smA, snap && s == 0);

            float dryL = left[s];
            float dryR = right[s];
            float inL = dryL;
            float inR = dryR;
            if (p.freeze)
            {
                inL = 0.f;
                inR = 0.f;
            }
            if (noiseS > 0.f)
            {
                inL += noiseS * pinkL.process (white.next());
                inR += noiseS * pinkR.process (white.next());
            }
            inL = hpL.hp (inL, hpS);
            inR = hpR.hp (inR, hpS);
            inL = lpL.lp (inL, lpS);
            inR = lpR.lp (inR, lpS);

            float wetL = 0.f, wetR = 0.f;
            for (int i = 0; i < kVoices; ++i)
            {
                if (! on[i])
                    continue;
                const bool snap0 = snap && s == 0;
                const float gLs = smoothToward (smGL[i], gL[i], smA, snap0);
                const float gRs = smoothToward (smGR[i], gR[i], smA, snap0);
                const float kS = smoothToward (smK[i], k[i], smA, snap0);
                const float gnS = smoothToward (smGain[i], gain[i], smA, snap0);
                const float pan = smoothToward (smPan[i], lastPan[i], smA, snap0);
                const float bpL = voices[i].l.band (inL, gLs, kS);
                const float bpR = voices[i].r.band (inR, gRs, kS);
                const float gLpan = std::min (1.f, 1.f - pan) * gnS;
                const float gRpan = std::min (1.f, 1.f + pan) * gnS;
                wetL += bpL * gLpan;
                wetR += bpR * gRpan;
                const float mag = std::max (std::abs (bpL), std::abs (bpR)) * gnS;
                auto& e = voices[i].energy;
                e = mag > e ? mag : e * 0.999f;
            }

            wetL = colorL.lp (wetL, colorS);
            wetR = colorR.lp (wetR, colorS);
            float outL = dryL * (1.f - mixS) + wetL * mixS;
            float outR = dryR * (1.f - mixS) + wetR * mixS;
            outL = softclip (outL * outS);
            outR = softclip (outR * outS);
            left[s] = outL;
            right[s] = outR;
        }

        for (int i = 0; i < kVoices; ++i)
            lastEnergy[i] = voices[i].energy;
    }
};

} // namespace fr3z0
