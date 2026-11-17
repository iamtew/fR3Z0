# Resonator VST3 Plugin  
## Visualizers & Graphics System

**Companion document to** `Resonator_VST3_Core_Intermediate_LFOs.md`

**Purpose**  
This document defines a multi-layered, useful visualization and graphics system for the resonator plugin.  
The goal is to go far beyond a basic spectrum analyzer so that users can clearly see:

- How the resonators are shaping the sound
- Which resonators are active and how strongly they are ringing
- How modulation is moving parameters
- Stereo width and spatial behaviour
- Changes over time

Good visuals increase understanding, speed up sound design, and make the plugin feel modern and controllable.

---

## Design Philosophy

- The main visualizer should be both a **display** and a **controller**.
- Information is organized in **toggleable layers** so the interface never becomes overwhelming.
- Visuals should teach the user how the resonator works while they use it.
- Prioritize clarity and usefulness over pure spectacle.
- Support progressive complexity: simple and clean by default, deeper layers available on demand.

---

## 1. Primary View — Interactive Resonator Graph

This is the central and most important visual element of the plugin. It replaces a conventional spectrum analyzer.

### Layers shown in the graph

**Background Layer**
- Real-time input spectrum (dimmed or switchable).
- Optional pre/post processing spectrum.

**Resonator Response Layer**
- Frequency response curve of every active resonator (steep peaks or comb-style teeth).
- Peak height represents current gain / intensity.
- Peak width or sharpness represents Q / steepness.

**Energy / Activity Layer**
- Visual indication of how strongly each resonator is currently ringing.
- Implementation options:
  - Glow or brightness on the peak
  - Vertical energy fill or bar above the base response
  - Soft shimmer / particle effect when strongly excited

**Modulation Ghosts / Trails**
- Semi-transparent ghost peaks or trails that move when frequency, gain, or Q is modulated.
- Instantly shows the range and direction of modulation.

**Stereo Information**
- Color coding or small L/R offsets to show stereo contribution.
- Optional mid-side coloring.

**Musical Overlay (toggleable)**
- Note names, piano-key style markings, or scale degrees.
- Helps users place resonators musically.

### Interaction

- Click empty space → add a new resonator
- Drag a peak → change frequency and/or gain
- Scroll wheel or side handles → adjust Q / steepness
- Right-click or modifier keys → delete, solo, or mute
- Hover → tooltip with exact frequency, note name + cents, gain, Q, pan, and current modulation amount

This graph is the primary place where users both understand and control the resonators.

---

## 2. Resonance Activity & Decay Visualizer

Answers the questions:  
“Which resonators are alive right now?” and “How long will they keep ringing?”

### Recommended elements

- Per-voice activity meters (horizontal bars, circular indicators, or vertical meters).
- Real-time lighting and decay that matches the actual resonator decay.
- Global “Resonance Energy” meter showing overall ringing amount.
- Optional short decay trails or fading history behind each meter.

Especially valuable when using long decays, freeze, or complex modulation.

---

## 3. Modulation Visualization

Make modulation visible instead of hidden.

### Per-parameter indicators
- Small animated arc, ring, or bar around every modulatable control.
- Shows current modulation depth and direction in real time.
- Pulses or moves when an LFO or envelope is active.

### Source displays
- Compact real-time LFO waveform views with playhead.
- Envelope follower level meters.
- Clear visual identity for each modulation source (color coding recommended).

### Optional advanced elements
- Lightweight connection lines or flowing particles from sources to destinations (subtle, can be toggled off).
- Mini sparkline / history trace next to key parameters showing recent modulated movement.

---

## 4. Stereo & Spatial View

Directly supports the goal of going from super-narrow to super-wide.

### Elements
- Horizontal stereo field: each resonator appears as a positioned dot or small peak according to its pan.
- Global width / spread visualization (expanding field or correlation-style display).
- Optional simple vectorscope or Lissajous of the wet signal.
- Color or brightness indicating left vs right energy contribution.
- Visual cues when stereo detune or micro-delay (Haas) is active.

This turns abstract width controls into something the user can see and sculpt.

---

## 5. Time-based / Historical Layers

Add a sense of movement and evolution over time.

### Recommended
- Narrow scrolling history strip or mini-spectrogram under/beside the main graph (last 1–4 seconds of resonant activity).
- Optional tilted waterfall / 3D-style history for a more dramatic view (can be heavier; make it optional).
- Global Freeze button that freezes all visualizers at once for detailed inspection.

---

## 6. Layer System & User Control

Do not show every layer at once.

### Controls
- Toggle buttons or icons for each major layer:
  - Input Spectrum
  - Resonator Responses
  - Energy / Activity
  - Modulation Ghosts
  - Stereo Information
  - Note / Scale Grid
  - History Strip
- Opacity or intensity sliders for individual layers.
- View presets / modes:
  - Simple
  - Sound Design
  - Modulation Focus
  - Stereo Focus
  - Analysis

This keeps the interface clean for beginners while giving power users full control.

---

## 7. Smart Behaviour & Feedback Details

Small touches that greatly increase understanding:

- Changing Global Root or Spread briefly highlights all affected resonators.
- Soloing a resonator dims everything else and focuses activity meters.
- Incoming MIDI notes show temporary markers on the frequency axis.
- Learn / peak-detect mode uses clear animated placement of new resonators.
- Subtle output vs dry comparison options (overlay or A/B flash).

---

## 8. Implementation Guidelines

### Technical recommendations
- Prefer OpenGL or GPU-accelerated / vector rendering in JUCE for smooth animation (trails, glows, history).
- Run visual analysis and UI updates at 30–60 fps, not full audio rate.
- Share filter state, envelope followers, and FFT data carefully between audio and message threads.
- Pre-compute or efficiently approximate resonator response curves so the main graph stays light.
- Keep the Interactive Resonator Graph the largest and most prominent element.

### Performance considerations
- Make heavier features (waterfall, flowing particles, high-resolution history) optional or quality-scalable.
- Provide a “Low CPU Visuals” mode if needed.

---

## 9. Suggested Implementation Priority

1. Interactive Resonator Graph with response curves + energy indication + basic drag interaction  
2. Per-voice activity / decay meters  
3. Modulation amount indicators on parameters + compact LFO waveform displays  
4. Stereo field visualization  
5. Toggleable input spectrum + note/scale overlay  
6. Scrolling history / mini-spectrogram  
7. Modulation ghosts/trails and optional connection visuals  
8. Full layer toggles, opacity controls, and view presets  

---

## 10. Relationship to Other Documents

- Use this document together with `Resonator_VST3_Core_Intermediate_LFOs.md`.
- The Interactive Resonator Graph directly supports the Intermediate-layer spectrum view and per-voice controls.
- Modulation visuals support the LFO section and prepare the ground for the full modulation matrix described in the Advanced document.
- Stereo visuals directly serve the width / narrowness goals defined in the core design.

---

This visualization system turns the plugin into a tool that both sounds good and helps the user understand and control complex resonant behaviour.

*Document version: 1.0 — Visualizers & Graphics*