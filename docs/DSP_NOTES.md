# DSP notes

Bandpass is a Cytomic TPT SVF (`Svf::band`). Q comes from intensity × voice Q, damped by decay so high decay still rings but does not self-oscillate forever.

Input HP/LP and Color are one-pole. Noise is Paul Kellet pink. Output is `tanh` soft clip.

Freeze zeros the input into the bank; filter state keeps decaying. Kill resets SVF/one-pole state (rising-edge bool or the Kill button).

Continuous coeffs (mix, output, noise, HP/LP, color, per-voice g/k/gain/pan) one-pole toward the block target at ~20 Hz. Kill / prepare snap.

LFO runs at block rate. Phase offsets the waveform (and S&H wrap). Sync uses host BPM × division.

MIDI note-on sets root for the current block, then writes the Root parameter from the message thread.
