# Resonator VST3 Plugin  
## Remaining Advanced Layer Features (Future Roadmap)

**Purpose of this document**  
This file contains the Advanced features that were **not** included in the Core + Intermediate + LFOs document.  
Keep this as a reference for future development once the foundation is solid and musical.

---

## 1. Full Modulation System (Beyond Basic LFOs)

### 1.1 Additional Modulation Sources
- Envelope Followers (with adjustable attack, release, sensitivity, and optional sidechain input)
- Multi-stage envelopes / DAHDSR
- Step sequencers (8–16 steps, with per-step values and direction modes)
- Random / chaos generators
- Macro controls (4–8 macros that can morph groups of parameters)
- MIDI CC and aftertouch as modulation sources
- Audio-rate modulation options for selected parameters (careful with CPU and stability)

### 1.2 Modulation Matrix
- Full source → destination matrix
- Bipolar and unipolar amounts
- Per-destination modulation depth
- Ability to modulate almost everything:
  - Global and per-voice frequency
  - Decay / feedback
  - Q / Steepness
  - Gain
  - Pan
  - Stereo spread / detune
  - Dry/Wet
  - Filter cutoffs
  - LFO rates and depths (modulation of modulation)
- Visual matrix or clean list-based assignment UI
- Modulation indicators / meters on parameters

### 1.3 Advanced Routing Ideas
- Per-voice modulation buses
- Modulation that can be delayed or lagged
- Conditional modulation (e.g. only when input is above a threshold)

---

## 2. Advanced MIDI & Performance Features

### 2.1 Polyphonic MIDI
- True polyphonic operation (multiple simultaneous notes)
- Voice allocation modes (newest, oldest, lowest, highest, round-robin)
- Voice stealing options
- Per-note decay / intensity scaling

### 2.2 MPE (MIDI Polyphonic Expression)
- Full MPE support
- Per-note pitch bend, pressure (aftertouch), and slide/timbre
- Mapping of MPE dimensions to resonator parameters (frequency, intensity, decay, pan, etc.)

### 2.3 Additional MIDI Features
- Glide / portamento (global and per-voice)
- MIDI learn on all meaningful parameters
- Note range / key tracking filters
- Sustain pedal support
- Ability to use the plugin as a playable resonant instrument (internal noise or weak input + MIDI)

---

## 3. Advanced Resonator Behavior & Models

### 3.1 Multiple Resonator Types / Models
- Classic comb / delay-line resonators (Ableton-style)
- Steep bandpass bank (current recommended foundation)
- Formant-style overlapping resonators (vocal character)
- Physical-model inspired responses (simple string/plate approximations)
- Morphing between models

### 3.2 Harmonic / Partial Control
- Generate many partials from each base resonator (up to 64–256 depending on CPU)
- Controls for:
  - Harmonic density / count
  - Stretch / ratio
  - Even / odd / custom partial selection
  - High-frequency damping of partials
  - Blur / spectral smearing of partials
- Independent control of fundamental vs overtone balance

### 3.3 Per-Resonator Envelopes
- Independent attack and release (or full AHDSR) per voice
- Different damping behavior for different frequency ranges

### 3.4 Stability & Character Options
- Multiple saturation / non-linearity stages inside the resonators
- Different feedback topologies
- “Character” presets or algorithms (clean, vintage, aggressive, soft, etc.)

---

## 4. Spectral & Creative Processing

### 4.1 Spectral Domain Options
- Freeze of the resonant state
- Spectral blur / smear
- Frequency shift of the entire resonator bank
- Spectral gating or thresholding
- Ability to process only a selected frequency region of the input

### 4.2 Creative Effects
- Vocoder-like behavior (using resonators as the filter bank)
- Remixing / re-synthesis style processing
- Granular-inspired excitation of the resonators
- Feedback paths that include pitch shifting or other processing

### 4.3 Analysis Features
- Real-time spectrum display with resonator overlays
- Peak tracking / following
- Onset detection to re-trigger or excite resonators
- Optional output of spectral data (for advanced users or visualizations)

---

## 5. Generative & Learning Tools

### 5.1 Spectrum Learning
- Capture a target spectrum (hold + analyze)
- Automatically place and gain-scale resonators to match prominent peaks
- “Learn Variation” that creates related but different banks
- Morph between two learned or manually created states

### 5.2 Generative Tools
- Random resonator bank generator (with musical constraints: scale, harmonic series, density, etc.)
- Mutate current bank
- Generate chord progressions or evolving sequences of resonator states
- Probability-based or rule-based generation

### 5.3 Preset & State Management
- Morphable presets
- Snapshot / A-B-C-D states with morphing
- Interpolation between any two complete resonator configurations

---

## 6. Advanced Stereo & Spatial Features

- Independent left and right resonator banks
- Mid-Side processing of the resonant signal
- Stereo image rotation or movement
- Frequency-dependent stereo width
- Binaural or simple spatial positioning options
- Advanced mono-compatibility tools and correlation metering
- Extreme width modes with safety limiting

These expand the “super wide ↔ super narrow” capabilities far beyond the Intermediate layer.

---

## 7. UI / UX Polish for Advanced Use

- Collapsible Advanced panel or separate pages
- Visual modulation matrix
- Per-voice detailed editor (pop-out or dedicated section)
- Real-time modulation amount visualization
- Spectrum view that also acts as a control surface (drag frequencies, gains, etc.)
- Customizable UI scaling and color themes
- Comprehensive tooltips and a built-in interactive guide / tutorial mode
- Gesture support and good MIDI controller integration

---

## 8. Performance, Quality & Utility Features

- Oversampling options for the non-linear stages
- Quality modes (Draft / Normal / High)
- CPU usage display and voice limiting under heavy load
- Detailed metering (input, output, resonance energy, stereo correlation)
- Undo / redo for graph and parameter changes
- A/B comparison
- Full DAW automation support with clear parameter names
- State saving that includes modulation assignments and learned spectra

---

## 9. Suggested Future Development Order (After Core + Intermediate + LFOs)

1. Envelope followers + expand the modulation matrix
2. True polyphonic MIDI + basic voice allocation
3. Improved harmonic/partial generation
4. Spectrum Learn + simple generative tools
5. MPE support
6. Additional resonator models / characters
7. Advanced stereo / mid-side features
8. Step sequencers and macros
9. Spectral freeze / blur / shift
10. Full polish, theming, and utility features

---

## 10. Design Philosophy Reminders

- Every advanced feature should remain **optional**. The Core experience must stay fast and musical.
- Prefer progressive disclosure in the UI (simple by default, depth on demand).
- Keep CPU reasonable — offer quality trade-offs and smart voice management.
- Always protect against runaway resonance (limiters, soft clipping, careful coefficient design).
- Make extreme settings (super wide, super narrow, very long decays, high Q) safe and usable.

---

This document is intentionally a high-level roadmap and feature inventory.  
When you are ready to implement any of these sections, break them down into concrete parameters, DSP approaches, and UI sketches.

---

*Document version: 1.0 — Advanced / Future features only*