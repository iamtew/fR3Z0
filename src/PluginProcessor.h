#pragma once

#include "dsp/Bank.h"
#include "parameters/ParameterLayout.h"

#include <JuceHeader.h>
#include <atomic>

class fR3Z0AudioProcessor : public juce::AudioProcessor
{
public:
    fR3Z0AudioProcessor();
    ~fR3Z0AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "fR3Z0"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    bool supportsMPE() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return fr3z0::kPresetCount; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void requestKill() { killFlag.store (true, std::memory_order_relaxed); }
    void setSelectedVoice (int i) { selected.store (juce::jlimit (0, fr3z0::kVoices - 1, i), std::memory_order_relaxed); }
    int selectedVoice() const { return selected.load (std::memory_order_relaxed); }

    juce::AudioProcessorValueTreeState apvts;
    fr3z0::Bank bank;

    std::atomic<int> selected { 0 };

    void copyViz (float* hz, float* q, float* pan, float* gain, float* energy, float* lfoOut) const;
    void copySpectrum (float* dest, int n) const;

private:
    void applyPreset (int index);
    float loadf (const char* id, float fallback = 0.f) const;
    bool loadb (const char* id) const;
    int loadi (const char* id) const;

    int currentProgram = 0;
    std::atomic<bool> killFlag { false };
    float midiBend = 0.f; // semitones
    float bpm = 120.f;

    juce::dsp::FFT fft { fr3z0::kFftOrder };
    alignas (16) float fftWork[fr3z0::kFftSize * 2] {};
    float specMag[fr3z0::kFftSize / 2] {};
    int fifoPos = 0;
    float fifo[fr3z0::kFftSize] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (fR3Z0AudioProcessor)
};
