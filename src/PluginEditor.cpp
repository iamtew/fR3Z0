#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr int kEditorW = 980;
constexpr int kEditorH = 640;
constexpr int kPad = 8;
}

using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

void fR3Z0AudioProcessorEditor::PanelLnF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                            float pos, float startAngle, float endAngle,
                                                            juce::Slider&)
{
    using namespace fr3z0;
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (1.f);
    const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
    if (d < 8.f) return;
    auto area = bounds.withSizeKeepingCentre (d, d);
    const auto c = area.getCentre();
    g.setColour (kCyan);
    for (int i = 0; i < 11; ++i)
    {
        const float t = (float) i / 10.f;
        const float a = startAngle + t * (endAngle - startAngle);
        const auto p0 = c.getPointOnCircumference (d * 0.48f, a);
        const auto p1 = c.getPointOnCircumference (d * 0.40f, a);
        g.drawLine (p0.x, p0.y, p1.x, p1.y, juce::jmax (1.f, d * 0.018f));
    }
    const float bodyD = d * 0.72f;
    auto body = area.withSizeKeepingCentre (bodyD, bodyD);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (body.translated (0.f, d * 0.018f));
    g.setColour (kMagenta.darker (0.25f));
    g.fillEllipse (body);
    auto cap = area.withSizeKeepingCentre (bodyD * 0.5f, bodyD * 0.5f);
    g.setColour (kMagenta);
    g.fillEllipse (cap);
    const float angle = startAngle + pos * (endAngle - startAngle);
    juce::Path pointer;
    pointer.startNewSubPath (c.getPointOnCircumference (bodyD * 0.32f, angle));
    pointer.lineTo (c.getPointOnCircumference (bodyD * 0.46f, angle));
    g.setColour (kCyan);
    g.strokePath (pointer, juce::PathStrokeType (juce::jmax (1.15f, d * 0.028f),
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

void fR3Z0AudioProcessorEditor::PanelLnF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                                            bool, bool)
{
    using namespace fr3z0;
    auto r = b.getLocalBounds().toFloat().reduced (2.f);
    g.setColour (b.getToggleState() ? kMagenta : kViolet);
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (kLight);
    g.setFont (12.f);
    g.drawFittedText (b.getButtonText(), r.toNearestInt(), juce::Justification::centred, 1);
}

juce::Font fR3Z0AudioProcessorEditor::PanelLnF::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions (11.f));
}

juce::Font fR3Z0AudioProcessorEditor::PanelLnF::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (12.f));
}

void fR3Z0AudioProcessorEditor::PresetBar::paint (juce::Graphics& g)
{
    using namespace fr3z0;
    auto r = getLocalBounds().toFloat();
    g.setColour (kViolet);
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (kLight);
    g.setFont (13.f);
    g.drawFittedText ("<  " + text + "  >", getLocalBounds(), juce::Justification::centred, 1);
}

void fR3Z0AudioProcessorEditor::PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (e.x < getWidth() / 5 && onStep) onStep (-1);
    else if (e.x > getWidth() * 4 / 5 && onStep) onStep (1);
    else if (onOpen) onOpen();
}

void fR3Z0AudioProcessorEditor::PresetBar::setText (const juce::String& t)
{
    text = t;
    repaint();
}

float fR3Z0AudioProcessorEditor::xToHz (float x, float w)
{
    const float t = juce::jlimit (0.f, 1.f, x / juce::jmax (1.f, w));
    return 20.f * std::pow (fr3z0::kMaxHz / 20.f, t);
}

float fR3Z0AudioProcessorEditor::hzToX (float hz, float w)
{
    const float t = std::log (juce::jmax (20.f, hz) / 20.f) / std::log (fr3z0::kMaxHz / 20.f);
    return t * w;
}

float fR3Z0AudioProcessorEditor::yToGain (float y, float h)
{
    return juce::jlimit (0.f, 2.f, (1.f - y / juce::jmax (1.f, h)) * 2.f);
}

float fR3Z0AudioProcessorEditor::gainToY (float g, float h)
{
    return (1.f - juce::jlimit (0.f, 2.f, g) / 2.f) * h;
}

int fR3Z0AudioProcessorEditor::hitVoice (juce::Point<float> p, juce::Rectangle<float> area) const
{
    int best = -1;
    float bestD = 18.f;
    for (int i = 0; i < fr3z0::kVoices; ++i)
    {
        if (vizGain[i] <= 0.001f && vizHz[i] <= 0.f)
            continue;
        if (auto* on = proc.apvts.getRawParameterValue (ParamID::vOn[i]))
            if (on->load() < 0.5f)
                continue;
        const float x = area.getX() + hzToX (vizHz[i], area.getWidth());
        const float y = area.getY() + gainToY (vizGain[i], area.getHeight());
        const float d = p.getDistanceFrom ({ x, y });
        if (d < bestD) { bestD = d; best = i; }
    }
    return best;
}

void fR3Z0AudioProcessorEditor::Graph::paint (juce::Graphics& g)
{
    using namespace fr3z0;
    auto r = getLocalBounds().toFloat();
    g.setColour (kWell);
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (kViolet);
    g.drawRoundedRectangle (r.reduced (0.5f), 6.f, 1.f);

    auto area = r.reduced (8.f);

    const bool specOn = ed.proc.apvts.getRawParameterValue (ParamID::spectrum)
                        && ed.proc.apvts.getRawParameterValue (ParamID::spectrum)->load() > 0.5f;
    if (specOn)
    {
        juce::Path sp;
        const int bins = 256;
        for (int i = 1; i < bins; ++i)
        {
            const float hz = xToHz ((float) i / (float) bins * area.getWidth(), area.getWidth());
            const float bin = hz / (ed.proc.getSampleRate() > 0 ? (float) ed.proc.getSampleRate() : 44100.f)
                              * (float) (fr3z0::kFftSize / 2);
            const int b = juce::jlimit (0, 255, (int) bin);
            const float mag = juce::jlimit (0.f, 1.f, ed.spec[b] * 40.f);
            const float x = area.getX() + area.getWidth() * (float) i / (float) bins;
            const float y = area.getBottom() - mag * area.getHeight();
            if (i == 1) sp.startNewSubPath (x, y);
            else sp.lineTo (x, y);
        }
        g.setColour (kBlue.withAlpha (0.45f));
        g.strokePath (sp, juce::PathStrokeType (1.2f));
    }

    const int sel = ed.proc.selectedVoice();
    for (int i = 0; i < kVoices; ++i)
    {
        if (auto* on = ed.proc.apvts.getRawParameterValue (ParamID::vOn[i]))
            if (on->load() < 0.5f)
                continue;
        const float hz = ed.vizHz[i] > 1.f ? ed.vizHz[i] : 20.f;
        const float x = area.getX() + hzToX (hz, area.getWidth());
        const float y = area.getY() + gainToY (ed.vizGain[i], area.getHeight());
        const float q = juce::jmax (0.5f, ed.vizQ[i]);
        const float w = juce::jmax (6.f, 140.f / q);
        const float e = juce::jlimit (0.f, 1.f, ed.vizEnergy[i] * 2.f);
        juce::Colour col = (i == sel) ? kCyan : kMagenta;
        g.setColour (col.withAlpha (0.25f + e * 0.55f));
        g.fillEllipse (x - w * 0.5f, y - 10.f, w, 20.f);
        g.setColour (col);
        g.fillEllipse (x - 5.f, y - 5.f, 10.f, 10.f);
        g.setColour (kLight);
        g.setFont (10.f);
        g.drawText (juce::String (i + 1), (int) x - 8, (int) y - 22, 16, 12, juce::Justification::centred);
    }

    // Activity meters
    auto meters = area.removeFromRight (8.f);
    (void) meters;
    float mx = r.getRight() - 14.f;
    for (int i = 0; i < kVoices; ++i)
    {
        const float e = juce::jlimit (0.f, 1.f, ed.vizEnergy[i] * 2.f);
        g.setColour (kMagenta.withAlpha (0.2f + e * 0.8f));
        g.fillRect (mx, r.getY() + 8.f + (float) i * 12.f, 6.f, 10.f);
    }
}

void fR3Z0AudioProcessorEditor::Graph::mouseDown (const juce::MouseEvent& e)
{
    auto area = getLocalBounds().toFloat().reduced (8.f);
    const int hit = ed.hitVoice (e.position, area);
    if (e.mods.isPopupMenu())
    {
        if (hit >= 0)
            if (auto* p = ed.proc.apvts.getParameter (ParamID::vOn[hit]))
                p->setValueNotifyingHost (0.f);
        return;
    }
    if (hit >= 0)
    {
        ed.proc.setSelectedVoice (hit);
        ed.dragVoice = hit;
        return;
    }
    for (int i = 0; i < fr3z0::kVoices; ++i)
        if (auto* on = ed.proc.apvts.getRawParameterValue (ParamID::vOn[i]))
            if (on->load() < 0.5f)
            {
                if (auto* p = ed.proc.apvts.getParameter (ParamID::vOn[i]))
                    p->setValueNotifyingHost (1.f);
                ed.writeVoice (i, xToHz (e.position.x - area.getX(), area.getWidth()),
                               yToGain (e.position.y - area.getY(), area.getHeight()));
                ed.proc.setSelectedVoice (i);
                ed.dragVoice = i;
                return;
            }
}

void fR3Z0AudioProcessorEditor::Graph::mouseUp (const juce::MouseEvent&)
{
    ed.dragVoice = -1;
}

void fR3Z0AudioProcessorEditor::Graph::mouseDrag (const juce::MouseEvent& e)
{
    if (ed.dragVoice < 0) return;
    auto area = getLocalBounds().toFloat().reduced (8.f);
    ed.writeVoice (ed.dragVoice,
                   xToHz (e.position.x - area.getX(), area.getWidth()),
                   yToGain (e.position.y - area.getY(), area.getHeight()));
}

void fR3Z0AudioProcessorEditor::Graph::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    const int i = ed.proc.selectedVoice();
    if (auto* p = ed.proc.apvts.getParameter (ParamID::vQ[i]))
    {
        const float cur = p->convertFrom0to1 (p->getValue());
        p->setValueNotifyingHost (p->convertTo0to1 (juce::jlimit (0.2f, 2.f, cur + w.deltaY * 0.15f)));
    }
}

void fR3Z0AudioProcessorEditor::writeVoice (int i, float hz, float gain)
{
    float rootMidi = 60.f;
    if (auto* rp = proc.apvts.getRawParameterValue (ParamID::rootMidi))
        rootMidi = rp->load();
    const float midi = 69.f + 12.f * std::log2 (juce::jmax (20.f, hz) / 440.f);
    float semi = midi - rootMidi;
    if (auto* snapP = proc.apvts.getRawParameterValue (ParamID::snap))
        if (snapP->load() > 0.5f)
            if (auto* sc = proc.apvts.getRawParameterValue (ParamID::scale))
                semi = fr3z0::snapMidi (rootMidi + semi, juce::roundToInt (sc->load())) - rootMidi;
    semi = juce::jlimit (-24.f, 24.f, semi);
    if (auto* p = proc.apvts.getParameter (ParamID::vSemi[i]))
        p->setValueNotifyingHost (p->convertTo0to1 (semi));
    if (auto* p = proc.apvts.getParameter (ParamID::vGain[i]))
        p->setValueNotifyingHost (p->convertTo0to1 (juce::jlimit (0.f, 2.f, gain)));
}

fR3Z0AudioProcessorEditor::fR3Z0AudioProcessorEditor (fR3Z0AudioProcessor& p)
    : AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);
    content.addAndMakeVisible (graph);
    content.addAndMakeVisible (presetBar);
    content.addAndMakeVisible (kill);
    content.addAndMakeVisible (scaleBox);
    scaleBox.addItemList (juce::StringArray { "Chromatic", "Major", "Minor", "Harmonic" }, 1);
    bindCombo (scaleBox, scaleA, ParamID::scale);

    auto setupToggle = [this] (juce::ToggleButton& b, std::unique_ptr<BA>& a, const char* id, const char* name)
    {
        b.setButtonText (name);
        content.addAndMakeVisible (b);
        a = std::make_unique<BA> (proc.apvts, id, b);
    };
    setupToggle (freeze, freezeA, ParamID::freeze, "Freeze");
    setupToggle (snap, snapA, ParamID::snap, "Snap");
    setupToggle (spectrum, specA, ParamID::spectrum, "Spectrum");
    setupToggle (bypass, bypassA, ParamID::bypass, "Bypass");
    setupToggle (vOn, vOnA, ParamID::vOn[0], "On");
    setupToggle (vLink, vLinkA, ParamID::vLink[0], "Link");

    bindKnob (root, ParamID::rootMidi, "Root");
    bindKnob (decay, ParamID::decay, "Decay");
    bindKnob (intensity, ParamID::intensity, "Intensity");
    bindKnob (mix, ParamID::mix, "Mix");
    bindKnob (outGain, ParamID::outGain, "Out");
    bindKnob (width, ParamID::width, "Width");
    bindKnob (detune, ParamID::detune, "Detune");
    bindKnob (hp, ParamID::hpHz, "HP");
    bindKnob (lp, ParamID::lpHz, "LP");
    bindKnob (noise, ParamID::noise, "Noise");
    bindKnob (color, ParamID::color, "Color");
    bindKnob (vSemi, ParamID::vSemi[0], "Interval");
    bindKnob (vCents, ParamID::vCents[0], "Cents");
    bindKnob (vGain, ParamID::vGain[0], "Gain");
    bindKnob (vPan, ParamID::vPan[0], "Pan");
    bindKnob (vDecay, ParamID::vDecay[0], "V Decay");
    bindKnob (vQ, ParamID::vQ[0], "Q");

    kill.onClick = [this] { proc.requestKill(); };
    presetBar.onStep = [this] (int d) { stepPreset (d); };
    presetBar.onOpen = [this] { openPresetMenu(); };

    for (int i = 0; i < 2; ++i)
    {
        lShape[i].addItemList (juce::StringArray { "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
        lDiv[i].addItemList (juce::StringArray { "1/1", "1/2", "1/4", "1/8", "1/8T", "1/16T" }, 1);
        content.addAndMakeVisible (lShape[i]);
        content.addAndMakeVisible (lDiv[i]);
        bindCombo (lShape[i], lShapeA[i], ParamID::lShape[i]);
        bindCombo (lDiv[i], lDivA[i], ParamID::lDiv[i]);
        bindKnob (lRate[i], ParamID::lRate[i], "Rate");
        bindKnob (lDepth[i], ParamID::lDepth[i], "Depth");
        bindKnob (lRoot[i], ParamID::lRoot[i], "Root");
        bindKnob (lDecay[i], ParamID::lDecay[i], "Decay");
        bindKnob (lInt[i], ParamID::lInt[i], "Int");
        bindKnob (lWidth[i], ParamID::lWidth[i], "Width");
        bindKnob (lMix[i], ParamID::lMix[i], "Mix");
        lSync[i].setButtonText ("Sync");
        lUni[i].setButtonText ("Uni");
        content.addAndMakeVisible (lSync[i]);
        content.addAndMakeVisible (lUni[i]);
        lSyncA[i] = std::make_unique<BA> (proc.apvts, ParamID::lSync[i], lSync[i]);
        lUniA[i] = std::make_unique<BA> (proc.apvts, ParamID::lUni[i], lUni[i]);
    }

    logo = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);

    using namespace fr3z0;
    auto& lf = getLookAndFeel();
    lf.setColour (juce::ComboBox::backgroundColourId, kPanel);
    lf.setColour (juce::ComboBox::textColourId, kLight);
    lf.setColour (juce::ComboBox::outlineColourId, kViolet);
    lf.setColour (juce::PopupMenu::backgroundColourId, kPanel);
    lf.setColour (juce::PopupMenu::textColourId, kLight);
    lf.setColour (juce::TextButton::buttonColourId, kViolet);
    lf.setColour (juce::TextButton::textColourOffId, kLight);
    lf.setColour (juce::TextButton::textColourOnId, kLight);

    refreshBar();
    baseW = kEditorW;
    baseH = kEditorH;
    content.setSize (baseW, baseH);
    setResizable (true, true);
    setResizeLimits (baseW * 2 / 5, baseH * 2 / 5, baseW * 3 / 2, baseH * 3 / 2);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) baseW / (double) baseH);
    setSize (baseW, baseH);
    startTimerHz (30);
}

fR3Z0AudioProcessorEditor::~fR3Z0AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void fR3Z0AudioProcessorEditor::bindKnob (Knob& k, const char* id, const juce::String& name)
{
    k.s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
    k.l.setText (name, juce::dontSendNotification);
    k.l.setJustificationType (juce::Justification::centred);
    k.l.setColour (juce::Label::textColourId, fr3z0::kLight);
    content.addAndMakeVisible (k.s);
    content.addAndMakeVisible (k.l);
    k.a = std::make_unique<SA> (proc.apvts, id, k.s);
}

void fR3Z0AudioProcessorEditor::bindCombo (juce::ComboBox& c,
                                           std::unique_ptr<CA>& a,
                                           const char* id)
{
    content.addAndMakeVisible (c);
    a = std::make_unique<CA> (proc.apvts, id, c);
}

void fR3Z0AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (fr3z0::kBlack);
}

void fR3Z0AudioProcessorEditor::resized()
{
    if (baseW == 0) return;
    const float s = (float) getWidth() / (float) baseW;
    content.setTransform (juce::AffineTransform::scale (s));
    content.setBounds (0, 0, baseW, baseH);
}

void fR3Z0AudioProcessorEditor::paintContent (juce::Graphics& g)
{
    using namespace fr3z0;
    g.fillAll (kBlack);
    auto title = juce::Rectangle<int> (kPad, kPad, 200, 28);
    if (logo.isValid())
        g.drawImage (logo, title.removeFromLeft (28).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (kMagenta);
    g.setFont (boldFont (18.f));
    g.drawText ("fR3Z0", title, juce::Justification::centredLeft, false);

    // LFO dots
    g.setColour (kCyan);
    const float p0 = 0.5f + 0.5f * vizLfo[0];
    const float p1 = 0.5f + 0.5f * vizLfo[1];
    g.fillEllipse (760.f + p0 * 40.f, 18.f, 8.f, 8.f);
    g.setColour (kBlue);
    g.fillEllipse (880.f + p1 * 40.f, 18.f, 8.f, 8.f);
}

void fR3Z0AudioProcessorEditor::layoutContent()
{
    auto r = juce::Rectangle<int> (0, 0, baseW, baseH).reduced (kPad);
    auto top = r.removeFromTop (32);
    top.removeFromLeft (160);
    presetBar.setBounds (top.removeFromLeft (220).reduced (0, 2));
    freeze.setBounds (top.removeFromLeft (64).reduced (2, 2));
    kill.setBounds (top.removeFromLeft (56).reduced (2, 2));
    snap.setBounds (top.removeFromLeft (56).reduced (2, 2));
    spectrum.setBounds (top.removeFromLeft (76).reduced (2, 2));
    bypass.setBounds (top.removeFromLeft (64).reduced (2, 2));
    scaleBox.setBounds (top.removeFromLeft (110).reduced (2, 2));

    graph.setBounds (r.removeFromTop (250).reduced (0, 4));

    auto place = [] (Knob& k, juce::Rectangle<int> cell)
    {
        auto lab = cell.removeFromTop (14);
        k.l.setBounds (lab);
        k.s.setBounds (cell);
    };

    auto globals = r.removeFromTop (100);
    Knob* gk[] = { &root, &decay, &intensity, &mix, &outGain, &width, &detune, &hp, &lp, &noise, &color };
    const int gw = globals.getWidth() / 11;
    for (int i = 0; i < 11; ++i)
        place (*gk[i], globals.removeFromLeft (gw).reduced (2, 0));

    auto voice = r.removeFromTop (100);
    vOn.setBounds (voice.removeFromLeft (44).reduced (2, 24));
    vLink.setBounds (voice.removeFromLeft (48).reduced (2, 24));
    Knob* vk[] = { &vSemi, &vCents, &vGain, &vPan, &vDecay, &vQ };
    const int vw = voice.getWidth() / 6;
    for (int i = 0; i < 6; ++i)
        place (*vk[i], voice.removeFromLeft (vw).reduced (2, 0));

    auto lfos = r;
    for (int i = 0; i < 2; ++i)
    {
        auto col = lfos.removeFromLeft (lfos.getWidth() / (2 - i));
        auto row1 = col.removeFromTop (28);
        lShape[i].setBounds (row1.removeFromLeft (100).reduced (2, 2));
        lSync[i].setBounds (row1.removeFromLeft (50).reduced (2, 2));
        lDiv[i].setBounds (row1.removeFromLeft (80).reduced (2, 2));
        lUni[i].setBounds (row1.removeFromLeft (50).reduced (2, 2));
        Knob* lk[] = { &lRate[i], &lDepth[i], &lRoot[i], &lDecay[i], &lInt[i], &lWidth[i], &lMix[i] };
        const int lw = col.getWidth() / 7;
        for (int j = 0; j < 7; ++j)
            place (*lk[j], col.removeFromLeft (lw).reduced (2, 0));
    }
}

void fR3Z0AudioProcessorEditor::timerCallback()
{
    proc.copyViz (vizHz, vizQ, vizPan, vizGain, vizEnergy, vizLfo);
    proc.copySpectrum (spec, 256);
    const int sel = proc.selectedVoice();
    if (sel != attachedVoice && sel >= 0 && sel < fr3z0::kVoices)
    {
        vOnA.reset();
        vLinkA.reset();
        vSemi.a.reset();
        vCents.a.reset();
        vGain.a.reset();
        vPan.a.reset();
        vDecay.a.reset();
        vQ.a.reset();
        vOnA = std::make_unique<BA> (proc.apvts, ParamID::vOn[sel], vOn);
        vLinkA = std::make_unique<BA> (proc.apvts, ParamID::vLink[sel], vLink);
        vSemi.a = std::make_unique<SA> (proc.apvts, ParamID::vSemi[sel], vSemi.s);
        vCents.a = std::make_unique<SA> (proc.apvts, ParamID::vCents[sel], vCents.s);
        vGain.a = std::make_unique<SA> (proc.apvts, ParamID::vGain[sel], vGain.s);
        vPan.a = std::make_unique<SA> (proc.apvts, ParamID::vPan[sel], vPan.s);
        vDecay.a = std::make_unique<SA> (proc.apvts, ParamID::vDecay[sel], vDecay.s);
        vQ.a = std::make_unique<SA> (proc.apvts, ParamID::vQ[sel], vQ.s);
        attachedVoice = sel;
    }
    graph.repaint();
    refreshBar();
    repaint (760, 10, 200, 24);
}

void fR3Z0AudioProcessorEditor::refreshBar()
{
    presetBar.setText (proc.getProgramName (proc.getCurrentProgram()));
}

void fR3Z0AudioProcessorEditor::stepPreset (int delta)
{
    const int i = (proc.getCurrentProgram() + delta + fr3z0::kPresetCount) % fr3z0::kPresetCount;
    proc.setCurrentProgram (i);
    refreshBar();
}

void fR3Z0AudioProcessorEditor::openPresetMenu()
{
    juce::PopupMenu m;
    m.setLookAndFeel (&lnf);
    for (int i = 0; i < fr3z0::kPresetCount; ++i)
        m.addItem (i + 1, fr3z0::kPresetNames[i], true, i == proc.getCurrentProgram());
    juce::Component::SafePointer<fR3Z0AudioProcessorEditor> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (presetBar),
                     [safe] (int result)
                     {
                         if (safe == nullptr || result <= 0) return;
                         safe->proc.setCurrentProgram (result - 1);
                         safe->refreshBar();
                     });
}
