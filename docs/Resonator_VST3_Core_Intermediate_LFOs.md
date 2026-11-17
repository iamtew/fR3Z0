# Resonator VST3 Plugin  
## Bootstrap + Core Layer + Intermediate Layer + LFOs

**Purpose of this document**  
This file covers everything needed to get a solid, usable resonator effects plugin running: project bootstrap, the Core (simple) layer, the Intermediate layer, and LFOs from the Advanced layer.  
Use this as your primary implementation guide. The companion document covers the remaining Advanced features for later expansion.

---

## 1. What a Resonator Does (Quick Reference)

A resonator effect runs the input signal through one or more tuned resonant filters (typically steep bandpass filters or feedback delay lines / comb filters).  
Energy at or near the tuned frequencies rings and sustains.  

Typical results:
- Subtle body / tonal enhancement
- Harmonic chords from noise or drums
- Metallic or vocal-like textures
- Sustained drones and pads

Key controls that define the character:
- Tuning (root + intervals or free frequencies)
- Decay / feedback (how long it rings)
- Intensity / Q / resonance amount
- Dry/Wet mix
- Stereo placement and spread

---

## 2. Project Bootstrap

### Recommended Tech Stack
- **Framework**: JUCE (most common and mature choice for VST3 + AU + CLAP + Standalone)
- **Language**: C++17 or later
- **DSP**: Your own filters or a lightweight library (e.g. digidaw, or simple biquad / comb implementations)
- **UI**: JUCE’s built-in components + custom OpenGL or vector graphics for the spectrum view later
- **Target formats**: VST3 primary, then AU and CLAP
- **Sample rates**: Support 44.1 kHz – 192 kHz cleanly; process at the host rate

### Suggested Project Structure
```
ResonatorPlugin/
├── Source/
│   ├── PluginProcessor.h/.cpp      // Main audio processor
│   ├── PluginEditor.h/.cpp         // UI
│   ├── DSP/
│   │   ├── ResonatorVoice.h/.cpp   // Single resonator (bandpass or comb)
│   │   ├── ResonatorBank.h/.cpp    // Collection of voices
│   │   ├── LFO.h/.cpp
│   │   └── Filters/                // Biquad, comb, one-pole, etc.
│   ├── Modulation/
│   │   └── ModulationMatrix.h/.cpp // Start simple, expand later
│   └── Utils/
├── Resources/                      // Presets, icons, fonts
└── CMakeLists.txt or Projucer project
```

### Core Architecture Decisions (Make These Early)
- Process in mono or stereo? → Stereo from the start (with mono-compatible options).
- How many voices in the first version? → 6–8 is a good sweet spot (easy to expand later).
- Resonator type for v1:  
  - **Steep bandpass bank** (easier to control Q and gain independently, good for spectral feel)  
  - **or** Comb / feedback delay lines (classic Ableton-style ringing, very efficient).  
  Recommendation: Start with a high-quality resonant bandpass (or parallel bandpasses) for clearer control of steepness and intensity. You can add a comb mode later.
- Parameter smoothing: Use one-pole or linear smoothing on every continuous parameter that affects audio.
- Threading: Keep DSP completely lock-free. UI updates via timers or message thread.

### Essential Parameters to Expose Immediately
Global:
- Root Note / Frequency
- Global Decay
- Global Intensity / Resonance
- Dry/Wet
- Output Gain
- Stereo Spread / Width
- Input Filter Cutoff + Resonance (simple LP/HP)

Per voice (at least):
- Enable
- Interval (semitones) or absolute frequency
- Gain
- Pan
- Individual Decay (optional but useful)
- Individual Q / Steepness (optional)

---

## 3. Core Layer – Implementation Focus

Goal: A user can load the plugin, pick a root note, set a few intervals, adjust decay and mix, and get useful resonant results in under 30 seconds.

### 3.1 Resonator Voice
Each voice should contain:
- Tunable resonant filter (bandpass recommended for v1)
- Decay / feedback coefficient
- Gain
- Pan (or left/right gains)
- Optional individual Q

**Key behaviors**:
- When input energy is present near the tuned frequency → the filter rings.
- When input stops → the resonance decays according to the decay parameter.
- High intensity + high Q can approach self-oscillation (be careful with stability and loudness).

### 3.2 Resonator Bank
- Fixed array or small vector of voices (start with 6–8).
- Global root frequency that all interval-based voices follow.
- Ability to enable/disable individual voices.
- Optional “master” voice that others can be detuned relative to.

### 3.3 Signal Flow (Core)
```
Input
  → Optional Input Filter (LP/HP)
  → Resonator Bank (parallel voices)
  → Sum
  → Dry/Wet mix
  → Output Gain / Soft Clipper
  → Output
```

### 3.4 Stereo Handling (Core)
- Simple global Width control that scales panning of the voices.
- Per-voice pan knobs.
- Optional global stereo detune (very small opposite pitch offsets L/R) – extremely effective for width.

### 3.5 Must-Have Utilities
- Bypass
- Freeze (hold the current resonant state / stop updating from new input while letting decay continue or freeze completely)
- Kill / Reset (immediately silence hanging resonances)
- Soft clipper or limiter on the output to prevent runaway resonance from destroying speakers/ears

### 3.6 Presets
Ship with at least 8–12 factory presets that demonstrate:
- Subtle body enhancement
- Simple major/minor chords
- Metallic / industrial
- Long drones
- Narrow focused resonance
- Wide stereo pads

---

## 4. Intermediate Layer

Once the Core is stable and musical, add these features. They dramatically increase usefulness without requiring a full modulation system.

### 4.1 Per-Voice Deep Controls
Expose for each voice:
- Independent frequency (or interval + fine tune in cents)
- Gain
- Pan
- Decay
- Q / Steepness
- Enable

Provide “Link to Global Root” toggle per voice.

### 4.2 Scale & Harmonic Helpers
- Global scale / key selector (or root + mode).
- Snap-to-scale and snap-to-harmonic-series modes for the voice frequencies.
- Simple harmonic series generator (set fundamental → auto-fill voices with harmonics or selected partials).

### 4.3 Interactive Spectrum View (Highly Recommended)
- Frequency vs Gain graph.
- Click to add a resonator, drag to move (X = frequency, Y = gain), right-click or button to remove.
- Visual representation of current filter responses.
- Optional “Learn” / peak-detect from input spectrum (hold a moment of audio and place resonators on strongest peaks).

This single feature makes the plugin feel modern and fast to use.

### 4.4 Internal Noise / Excitation Source
- Blendable pink or white noise (or simple filtered noise).
- Allows the plugin to generate sound with no input (great for drones and testing).
- Can also act as a gentle excitation for very quiet inputs.

### 4.5 Better Stereo Imaging
- Global Stereo Detune (cents)
- Global Spread / Width (with mono-compat option)
- Per-voice pan + optional independent L/R processing
- Simple Haas micro-delay option for extra width

These controls are what let users go from “super narrow / laser focused” to “super wide”.

### 4.6 Basic MIDI Support
- Receive MIDI notes to set the global root (monophonic).
- Optional: map incoming notes to individual voices (simple polyphonic assignment).
- Pitch bend support (global or per-voice range).
- Velocity can modulate intensity or decay if desired.

### 4.7 Input / Output Tone Shaping
- Pre-resonator filter (LP/HP or simple EQ)
- Post-resonator high-frequency damping / Color control
- Optional subtle saturation / drive stages with a couple of characters

---

## 5. LFOs (from the Advanced Layer)

Implement a solid LFO system early — it is one of the highest-value modulation sources and is relatively self-contained.

### Recommended LFO Features (v1)
- 2–4 independent LFOs
- Waveforms: Sine, Triangle, Saw, Square, Random / Sample & Hold
- Rate: free (Hz) + tempo-sync (note values)
- Depth / Amount
- Phase offset
- Optional unipolar / bipolar switch
- Optional fade-in time

### Targets (start with these)
- Global Frequency / Root
- Global Decay
- Global Intensity / Resonance
- Stereo Spread / Width
- Dry/Wet
- Per-voice Frequency, Gain, Pan, Decay (via a simple routing system)

### Implementation Notes
- Run LFOs at audio rate or block rate (block rate is usually sufficient and cheaper).
- Provide a simple modulation matrix or per-parameter “Mod Amount” knobs that can select which LFO is the source.
- Visual feedback (small waveform displays or moving indicators) is very helpful.

Later you can expand this into a full matrix with envelope followers, step sequencers, etc. Getting good LFOs in early makes the plugin feel alive.

---

## 6. Implementation Priority Order

1. Project bootstrap + basic stereo audio I/O + parameter system
2. Single resonator voice (bandpass or comb) that works and sounds good
3. Resonator bank (6–8 voices) + global root + basic per-voice controls
4. Dry/Wet, Output Gain, simple stereo width/pan
5. Decay behavior that feels musical
6. Basic UI with the essential knobs
7. Presets + Freeze/Kill
8. Per-voice deep controls + scale/harmonic helpers
9. Internal noise source
10. Interactive spectrum view (big usability win)
11. Basic MIDI note → root
12. LFOs + simple modulation routing
13. Improved stereo imaging (detune, Haas, etc.)

---

## 7. Quality & Stability Checklist

- All continuous parameters are smoothed
- No zipper noise
- Resonance remains stable at high Q / high intensity (limit maximum feedback or use careful coefficient calculation)
- Output never explodes (soft clipper + optional brickwall limiter)
- Low CPU when few voices are active
- Clean behavior when changing sample rate or block size
- Good default preset that sounds immediately useful

---

## Next Steps After This Document

Once the Core + Intermediate + LFOs are solid and musical:
- Move to the companion document for the remaining Advanced features (full modulation matrix, polyphonic/MPE MIDI, spectral processing, generative tools, etc.).

This foundation already gives you a competitive, usable resonator plugin. Everything else is expansion.

---

*Document version: 1.0 — Core / Intermediate / LFOs focus*