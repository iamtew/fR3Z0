# Architecture

`PluginProcessor` owns APVTS and `fr3z0::Bank`. Each block copies parameter atomics into `BlockIn` (no alloc) and calls `bank.process`.

DSP math in `src/dsp/Units.h` is JUCE-free so `tests/fr3z0_check.cpp` can compile with `cl` only.

The editor scales a fixed 980×640 layout. Selected-voice knobs rebind APVTS attachments when the graph selection changes.

Viz: the bank writes `lastHz` / energy / LFO values and `lastLfoPhase`; the editor timer copies them at 30 fps. LFO wells draw one cycle plus a playhead. Optional FFT of the wet output when Spectrum is on.
