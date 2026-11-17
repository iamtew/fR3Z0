# Parameters

## Global

- **Root** — MIDI note the linked voices follow.
- **Decay** — how long the bank rings.
- **Intensity** — resonance / Q.
- **Mix** — dry/wet.
- **Out** — output gain into the soft clipper.
- **Width** — scales per-voice pan.
- **Detune** — slight L/R pitch offset in cents.
- **HP / LP** — input filter.
- **Noise** — pink-noise exciter.
- **Color** — post-bank tone (brighter is more HF).
- **Scale / Snap** — chromatic, major, minor, or harmonic series.
- **Freeze / Kill / Bypass / Spectrum**. Kill is a pulse (button or host automation rising edge) that zeros filter state.

## Per voice (selected strip + graph)

Enable, interval, cents, gain, pan, decay, Q, link-to-root.

Click a peak to select that voice. Click empty space to enable the next free slot. Right-click disables.
Drag writes interval and cents the same way the voice is tuned: vs root when linked, vs MIDI 60 when unlinked. Harmonic + link ignores X (frequency is the partial for that slot); Y still writes gain.

## LFOs

Two LFOs: shape, rate, phase, tempo sync, depth, unipolar. Amounts to root, decay, intensity, width, mix, and to all voices' frequency (VHz), gain (VGn), pan (VPn), and decay (VDc).
