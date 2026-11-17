#pragma once

#include "ParameterIDs.h"
#include "dsp/Presets.h"
#include "dsp/Units.h"

#include <JuceHeader.h>

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    using B = juce::AudioParameterBool;
    using C = juce::AudioParameterChoice;
    using ID = juce::ParameterID;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addF = [&] (const char* id, juce::String name, juce::NormalisableRange<float> r, float def)
    {
        layout.add (std::make_unique<P> (ID { id, 1 }, std::move (name), r, def));
    };
    auto addB = [&] (const char* id, juce::String name, bool def)
    {
        layout.add (std::make_unique<B> (ID { id, 1 }, std::move (name), def));
    };

    addF (ParamID::rootMidi, "Root", { 24.f, 96.f, 0.01f }, 60.f);
    addF (ParamID::decay, "Decay", { 0.f, 1.f, 0.001f }, 0.5f);
    addF (ParamID::intensity, "Intensity", { 0.f, 1.f, 0.001f }, 0.4f);
    addF (ParamID::mix, "Mix", { 0.f, 1.f, 0.001f }, 0.5f);
    addF (ParamID::outGain, "Output", { -24.f, 12.f, 0.01f }, 0.f);
    addF (ParamID::width, "Width", { 0.f, 1.f, 0.001f }, 0.5f);
    addF (ParamID::detune, "Detune", { 0.f, 20.f, 0.01f }, 0.f);
    addF (ParamID::hpHz, "HP", juce::NormalisableRange<float> (20.f, 2000.f, 0.1f, 0.4f), 20.f);
    addF (ParamID::lpHz, "LP", juce::NormalisableRange<float> (200.f, 18000.f, 1.f, 0.4f), 16000.f);
    addF (ParamID::noise, "Noise", { 0.f, 1.f, 0.001f }, 0.f);
    addF (ParamID::color, "Color", { 0.f, 1.f, 0.001f }, 0.7f);
    addB (ParamID::freeze, "Freeze", false);
    addB (ParamID::bypass, "Bypass", false);
    addB (ParamID::snap, "Snap", false);
    addB (ParamID::spectrum, "Spectrum", false);
    layout.add (std::make_unique<C> (ID { ParamID::scale, 1 }, "Scale",
                                     juce::StringArray { "Chromatic", "Major", "Minor", "Harmonic" }, 0));

    for (int i = 0; i < fr3z0::kVoices; ++i)
    {
        const auto n = juce::String (i + 1);
        addB (ParamID::vOn[i], "V" + n + " On", i == 0);
        addF (ParamID::vSemi[i], "V" + n + " Interval", { -24.f, 24.f, 0.01f }, 0.f);
        addF (ParamID::vCents[i], "V" + n + " Cents", { -50.f, 50.f, 0.1f }, 0.f);
        addF (ParamID::vGain[i], "V" + n + " Gain", { 0.f, 2.f, 0.001f }, i == 0 ? 0.8f : 0.7f);
        addF (ParamID::vPan[i], "V" + n + " Pan", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::vDecay[i], "V" + n + " Decay", { 0.f, 1.f, 0.001f }, 0.5f);
        addF (ParamID::vQ[i], "V" + n + " Q", { 0.2f, 2.f, 0.001f }, 1.f);
        addB (ParamID::vLink[i], "V" + n + " Link", true);
    }

    const juce::StringArray shapes { "Sine", "Triangle", "Saw", "Square", "S&H" };
    const juce::StringArray divs { "1/1", "1/2", "1/4", "1/8", "1/8T", "1/16T" };
    for (int i = 0; i < fr3z0::kLfos; ++i)
    {
        const auto n = juce::String (i + 1);
        layout.add (std::make_unique<C> (ID { ParamID::lShape[i], 1 }, "LFO" + n + " Shape", shapes, 0));
        addF (ParamID::lRate[i], "LFO" + n + " Rate",
              juce::NormalisableRange<float> (0.01f, 20.f, 0.001f, 0.4f), 0.5f);
        addB (ParamID::lSync[i], "LFO" + n + " Sync", false);
        layout.add (std::make_unique<C> (ID { ParamID::lDiv[i], 1 }, "LFO" + n + " Div", divs, 2));
        addF (ParamID::lDepth[i], "LFO" + n + " Depth", { 0.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lPhase[i], "LFO" + n + " Phase", { 0.f, 1.f, 0.001f }, 0.f);
        addB (ParamID::lUni[i], "LFO" + n + " Uni", false);
        addF (ParamID::lRoot[i], "LFO" + n + " to Root", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lDecay[i], "LFO" + n + " to Decay", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lInt[i], "LFO" + n + " to Intensity", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lWidth[i], "LFO" + n + " to Width", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lMix[i], "LFO" + n + " to Mix", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lVFreq[i], "LFO" + n + " to V Freq", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lVGain[i], "LFO" + n + " to V Gain", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lVPan[i], "LFO" + n + " to V Pan", { -1.f, 1.f, 0.001f }, 0.f);
        addF (ParamID::lVDecay[i], "LFO" + n + " to V Decay", { -1.f, 1.f, 0.001f }, 0.f);
    }

    return layout;
}
