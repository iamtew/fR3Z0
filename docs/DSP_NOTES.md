# DSP notes

Bandpass is a Cytomic TPT SVF (`Svf::band`). Q comes from intensity × voice Q, damped by decay so high decay still rings but does not self-oscillate forever.

Input HP/LP and Color are one-pole. Noise is Paul Kellet pink. Output is `tanh` soft clip.

Freeze zeros the input into the bank; filter state keeps decaying. Kill resets SVF/one-pole state.

LFO runs at block rate. Sync uses host BPM × division.
