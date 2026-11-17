#include "PluginProcessor.h"
#include "PluginEditor.h"

fR3Z0AudioProcessor::fR3Z0AudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

void fR3Z0AudioProcessor::prepareToPlay (double sampleRate, int)
{
    bank.prepare (sampleRate);
    fifoPos = 0;
}

bool fR3Z0AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

float fR3Z0AudioProcessor::loadf (const char* id, float fallback) const
{
    if (auto* p = apvts.getRawParameterValue (id))
        return p->load();
    return fallback;
}

bool fR3Z0AudioProcessor::loadb (const char* id) const
{
    return loadf (id) > 0.5f;
}

int fR3Z0AudioProcessor::loadi (const char* id) const
{
    return juce::roundToInt (loadf (id));
}

void fR3Z0AudioProcessor::applyPreset (int index)
{
    index = juce::jlimit (0, fr3z0::kPresetCount - 1, index);
    currentProgram = index;
    const auto& pr = fr3z0::kPresets[index];
    auto setF = [this] (const char* id, float v)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (v));
    };
    auto setB = [this] (const char* id, bool v)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (v ? 1.f : 0.f);
    };
    auto setC = [this] (const char* id, int v)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) v));
    };

    setF (ParamID::rootMidi, pr.rootMidi);
    setF (ParamID::decay, pr.decay);
    setF (ParamID::intensity, pr.intensity);
    setF (ParamID::mix, pr.mix);
    setF (ParamID::outGain, pr.outGainDb);
    setF (ParamID::width, pr.width);
    setF (ParamID::detune, pr.detune);
    setF (ParamID::hpHz, pr.hpHz);
    setF (ParamID::lpHz, pr.lpHz);
    setF (ParamID::noise, pr.noise);
    setF (ParamID::color, pr.color);
    setC (ParamID::scale, pr.scale);
    setB (ParamID::snap, pr.snap > 0.5f);
    setB (ParamID::freeze, false);
    setB (ParamID::kill, false);
    setB (ParamID::bypass, false);
    for (int i = 0; i < fr3z0::kVoices; ++i)
    {
        const auto& v = pr.v[i];
        setB (ParamID::vOn[i], v.on > 0.5f);
        setF (ParamID::vSemi[i], juce::jlimit (-24.f, 24.f, v.semi));
        setF (ParamID::vCents[i], v.cents);
        setF (ParamID::vGain[i], v.gain);
        setF (ParamID::vPan[i], v.pan);
        setF (ParamID::vDecay[i], v.decay);
        setF (ParamID::vQ[i], v.q);
        setB (ParamID::vLink[i], v.link > 0.5f);
    }
}

void fR3Z0AudioProcessor::setCurrentProgram (int index)
{
    applyPreset (index);
}

const juce::String fR3Z0AudioProcessor::getProgramName (int index)
{
    index = juce::jlimit (0, fr3z0::kPresetCount - 1, index);
    return fr3z0::kPresetNames[index];
}

void fR3Z0AudioProcessor::copyViz (float* hz, float* q, float* pan, float* gain, float* energy, float* lfoOut) const
{
    for (int i = 0; i < fr3z0::kVoices; ++i)
    {
        hz[i] = bank.lastHz[i];
        q[i] = bank.lastQ[i];
        pan[i] = bank.lastPan[i];
        gain[i] = bank.lastGain[i];
        energy[i] = bank.lastEnergy[i];
    }
    lfoOut[0] = bank.lastLfo[0];
    lfoOut[1] = bank.lastLfo[1];
}

void fR3Z0AudioProcessor::copySpectrum (float* dest, int n) const
{
    const int m = juce::jmin (n, fr3z0::kFftSize / 2);
    for (int i = 0; i < m; ++i)
        dest[i] = specMag[i];
}

void fR3Z0AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())
        {
            pendingRoot.store ((float) m.getNoteNumber(), std::memory_order_relaxed);
            triggerAsyncUpdate();
        }
        else if (m.isPitchWheel())
        {
            midiBend = (m.getPitchWheelValue() / 8192.f - 1.f) * 2.f;
        }
    }

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                bpm = (float) *b;

    const bool killPulse = killFlag.exchange (false, std::memory_order_relaxed);
    const bool killParam = loadb (ParamID::kill);
    if (killPulse || (killParam && ! lastKillParam))
        bank.reset();
    lastKillParam = killParam;

    if (loadb (ParamID::bypass))
        return;

    if (buffer.getNumChannels() == 1)
    {
        // Duplicate later; process as mono in both pointers.
    }

    const int n = buffer.getNumSamples();
    const int chans = buffer.getNumChannels();
    if (chans == 0 || n == 0)
        return;

    if (chans == 1)
    {
        // Make a second channel on the stack for the bank.
    }

    fr3z0::BlockIn in;
    const float pending = pendingRoot.load (std::memory_order_relaxed);
    const float root = pending >= 0.f ? pending : loadf (ParamID::rootMidi, 60.f);
    in.rootMidi = root + midiBend;
    in.decay = loadf (ParamID::decay, 0.5f);
    in.intensity = loadf (ParamID::intensity, 0.4f);
    in.mix = loadf (ParamID::mix, 0.5f);
    in.outGain = juce::Decibels::decibelsToGain (loadf (ParamID::outGain, 0.f));
    in.width = loadf (ParamID::width, 0.5f);
    in.detune = loadf (ParamID::detune, 0.f);
    in.hpHz = loadf (ParamID::hpHz, 20.f);
    in.lpHz = loadf (ParamID::lpHz, 16000.f);
    in.noise = loadf (ParamID::noise, 0.f);
    in.color = loadf (ParamID::color, 0.7f);
    in.freeze = loadb (ParamID::freeze);
    in.scale = loadi (ParamID::scale);
    in.snap = loadb (ParamID::snap);
    in.bpm = bpm;
    for (int i = 0; i < fr3z0::kVoices; ++i)
    {
        in.v[i].on = loadb (ParamID::vOn[i]);
        in.v[i].link = loadb (ParamID::vLink[i]);
        in.v[i].semi = loadf (ParamID::vSemi[i]);
        in.v[i].cents = loadf (ParamID::vCents[i]);
        in.v[i].gain = loadf (ParamID::vGain[i], 0.7f);
        in.v[i].pan = loadf (ParamID::vPan[i]);
        in.v[i].decay = loadf (ParamID::vDecay[i], 0.5f);
        in.v[i].q = loadf (ParamID::vQ[i], 1.f);
    }
    for (int i = 0; i < fr3z0::kLfos; ++i)
    {
        in.lfo[i].shape = loadi (ParamID::lShape[i]);
        in.lfo[i].rate = loadf (ParamID::lRate[i], 0.5f);
        in.lfo[i].sync = loadb (ParamID::lSync[i]);
        in.lfo[i].div = loadi (ParamID::lDiv[i]);
        in.lfo[i].depth = loadf (ParamID::lDepth[i]);
        in.lfo[i].phaseOff = loadf (ParamID::lPhase[i]);
        in.lfo[i].uni = loadb (ParamID::lUni[i]);
        in.lfo[i].aRoot = loadf (ParamID::lRoot[i]);
        in.lfo[i].aDecay = loadf (ParamID::lDecay[i]);
        in.lfo[i].aInt = loadf (ParamID::lInt[i]);
        in.lfo[i].aWidth = loadf (ParamID::lWidth[i]);
        in.lfo[i].aMix = loadf (ParamID::lMix[i]);
        in.lfo[i].aVFreq = loadf (ParamID::lVFreq[i]);
        in.lfo[i].aVGain = loadf (ParamID::lVGain[i]);
        in.lfo[i].aVPan = loadf (ParamID::lVPan[i]);
        in.lfo[i].aVDecay = loadf (ParamID::lVDecay[i]);
    }

    float* l = buffer.getWritePointer (0);
    float* r = chans > 1 ? buffer.getWritePointer (1) : l;
    if (chans == 1)
    {
        // Bank writes both; same pointer is OK for mono.
    }
    bank.process (l, r, n, in);

    if (loadb (ParamID::spectrum))
    {
        for (int i = 0; i < n; ++i)
        {
            fifo[fifoPos++] = 0.5f * (l[i] + (r == l ? l[i] : r[i]));
            if (fifoPos >= fr3z0::kFftSize)
            {
                fifoPos = 0;
                juce::FloatVectorOperations::copy (fftWork, fifo, fr3z0::kFftSize);
                juce::FloatVectorOperations::clear (fftWork + fr3z0::kFftSize, fr3z0::kFftSize);
                fft.performFrequencyOnlyForwardTransform (fftWork);
                const int bins = fr3z0::kFftSize / 2;
                for (int b = 0; b < bins; ++b)
                    specMag[b] = fftWork[b] * (1.f / (float) fr3z0::kFftSize);
            }
        }
    }
}

void fR3Z0AudioProcessor::handleAsyncUpdate()
{
    float n = pendingRoot.load (std::memory_order_relaxed);
    if (n < 0.f)
        return;
    if (auto* p = apvts.getParameter (ParamID::rootMidi))
        p->setValueNotifyingHost (p->convertTo0to1 (n));
    if (! pendingRoot.compare_exchange_strong (n, -1.f, std::memory_order_relaxed))
        triggerAsyncUpdate();
}

void fR3Z0AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void fR3Z0AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* fR3Z0AudioProcessor::createEditor()
{
    return new fR3Z0AudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new fR3Z0AudioProcessor();
}
