#pragma once

#include "PluginProcessor.h"
#include "ui/Theme.h"

#include <functional>

class fR3Z0AudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
    explicit fR3Z0AudioProcessorEditor (fR3Z0AudioProcessor&);
    ~fR3Z0AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Content : public juce::Component
    {
        explicit Content (fR3Z0AudioProcessorEditor& e) : ed (e) {}
        void paint (juce::Graphics& g) override { ed.paintContent (g); }
        void resized() override { ed.layoutContent(); }
        fR3Z0AudioProcessorEditor& ed;
    };

    struct PanelLnF : public juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                               float startAngle, float endAngle, juce::Slider&) override;
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
        juce::Font getLabelFont (juce::Label&) override;
        juce::Font getTextButtonFont (juce::TextButton&, int) override;
    };

    struct PresetBar : public juce::Component, public juce::SettableTooltipClient
    {
        void paint (juce::Graphics& g) override;
        void mouseUp (const juce::MouseEvent& e) override;
        void setText (const juce::String& t);
        juce::String text;
        std::function<void (int)> onStep;
        std::function<void()> onOpen;
    };

    class Graph : public juce::Component
    {
    public:
        explicit Graph (fR3Z0AudioProcessorEditor& e) : ed (e) {}
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
        fR3Z0AudioProcessorEditor& ed;
    };

    struct Knob
    {
        juce::Slider s;
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a;
    };

    void paintContent (juce::Graphics&);
    void layoutContent();
    void drawWell (juce::Graphics&, juce::Rectangle<int>, const juce::String& caption);
    void drawLfoWave (juce::Graphics&, juce::Rectangle<int>, int lfo);
    void timerCallback() override;
    void bindKnob (Knob& k, const char* id, const juce::String& name);
    void bindCombo (juce::ComboBox& c, std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& a,
                    const char* id);
    void stepPreset (int delta);
    void openPresetMenu();
    void refreshBar();
    void writeVoice (int i, float hz, float gain);
    int hitVoice (juce::Point<float> p, juce::Rectangle<float> area) const;
    static float xToHz (float x, float w);
    static float hzToX (float hz, float w);
    static float yToGain (float y, float h);
    static float gainToY (float g, float h);

    fR3Z0AudioProcessor& proc;
    PanelLnF lnf;
    Content content { *this };
    int baseW = 0, baseH = 0;

    Graph graph { *this };
    PresetBar presetBar;
    juce::Image logo;
    juce::TooltipWindow tip { this, 400 };

    Knob root, decay, intensity, mix, outGain, width, detune, hp, lp, noise, color;
    Knob vSemi, vCents, vGain, vPan, vDecay, vQ;
    juce::ToggleButton vOn, vLink, freeze, snap, spectrum, bypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vOnA, vLinkA, freezeA, snapA, specA, bypassA;
    juce::ComboBox scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleA;
    juce::TextButton kill { "Kill" };

    juce::ComboBox lShape[2], lDiv[2];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lShapeA[2], lDivA[2];
    Knob lRate[2], lDepth[2], lPhase[2], lRoot[2], lDecay[2], lInt[2], lWidth[2], lMix[2];
    Knob lVFreq[2], lVGain[2], lVPan[2], lVDecay[2];
    juce::ToggleButton lSync[2], lUni[2];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lSyncA[2], lUniA[2];

    juce::Rectangle<int> wellHeader, wellGraph, wellGlobal, wellVoice, wellLfo[2], wellLfoWave[2];
    int dragVoice = -1;
    int attachedVoice = -1;
    float vizHz[8] {}, vizQ[8] {}, vizPan[8] {}, vizGain[8] {}, vizEnergy[8] {}, vizLfo[2] {}, vizLfoPhase[2] {};
    float spec[512] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (fR3Z0AudioProcessorEditor)
};
