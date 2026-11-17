# SPEC

Resonator effect. VST3 + standalone. Projucer, JUCE 9.0.3, C++17, APVTS.

- Eight TPT bandpass voices, stereo, per-voice pan.
- Globals: root, decay, intensity, mix, output, width, detune, HP/LP, noise, color, freeze, kill, bypass, scale/snap.
- Two LFOs with destination amounts.
- Interactive graph writes the same parameters as the knobs.
- MIDI note-on → root. Pitch bend ±2 semitones.

Not in v1: CLAP, AU, CMake, full modulation matrix, MPE, waterfall visuals.
