#pragma once

namespace ParamID
{

inline constexpr const char* rootMidi   = "rootMidi";
inline constexpr const char* decay      = "decay";
inline constexpr const char* intensity  = "intensity";
inline constexpr const char* mix        = "mix";
inline constexpr const char* outGain    = "outGain";
inline constexpr const char* width      = "width";
inline constexpr const char* detune     = "detune";
inline constexpr const char* hpHz       = "hpHz";
inline constexpr const char* lpHz       = "lpHz";
inline constexpr const char* noise      = "noise";
inline constexpr const char* color      = "color";
inline constexpr const char* freeze     = "freeze";
inline constexpr const char* kill       = "kill";
inline constexpr const char* bypass     = "bypass";
inline constexpr const char* scale      = "scale";
inline constexpr const char* snap       = "snap";
inline constexpr const char* spectrum   = "spectrum";

inline constexpr const char* vOn[]     = { "v0_on", "v1_on", "v2_on", "v3_on", "v4_on", "v5_on", "v6_on", "v7_on" };
inline constexpr const char* vSemi[]   = { "v0_semi", "v1_semi", "v2_semi", "v3_semi", "v4_semi", "v5_semi", "v6_semi", "v7_semi" };
inline constexpr const char* vCents[]  = { "v0_cents", "v1_cents", "v2_cents", "v3_cents", "v4_cents", "v5_cents", "v6_cents", "v7_cents" };
inline constexpr const char* vGain[]   = { "v0_gain", "v1_gain", "v2_gain", "v3_gain", "v4_gain", "v5_gain", "v6_gain", "v7_gain" };
inline constexpr const char* vPan[]    = { "v0_pan", "v1_pan", "v2_pan", "v3_pan", "v4_pan", "v5_pan", "v6_pan", "v7_pan" };
inline constexpr const char* vDecay[]  = { "v0_decay", "v1_decay", "v2_decay", "v3_decay", "v4_decay", "v5_decay", "v6_decay", "v7_decay" };
inline constexpr const char* vQ[]      = { "v0_q", "v1_q", "v2_q", "v3_q", "v4_q", "v5_q", "v6_q", "v7_q" };
inline constexpr const char* vLink[]   = { "v0_link", "v1_link", "v2_link", "v3_link", "v4_link", "v5_link", "v6_link", "v7_link" };

inline constexpr const char* lShape[]  = { "l0_shape", "l1_shape" };
inline constexpr const char* lRate[]   = { "l0_rate", "l1_rate" };
inline constexpr const char* lSync[]   = { "l0_sync", "l1_sync" };
inline constexpr const char* lDiv[]    = { "l0_div", "l1_div" };
inline constexpr const char* lDepth[]  = { "l0_depth", "l1_depth" };
inline constexpr const char* lPhase[]  = { "l0_phase", "l1_phase" };
inline constexpr const char* lUni[]    = { "l0_uni", "l1_uni" };
inline constexpr const char* lRoot[]   = { "l0_root", "l1_root" };
inline constexpr const char* lDecay[]  = { "l0_decay", "l1_decay" };
inline constexpr const char* lInt[]    = { "l0_int", "l1_int" };
inline constexpr const char* lWidth[]  = { "l0_width", "l1_width" };
inline constexpr const char* lMix[]    = { "l0_mix", "l1_mix" };
inline constexpr const char* lVFreq[]  = { "l0_vfreq", "l1_vfreq" };
inline constexpr const char* lVGain[]  = { "l0_vgain", "l1_vgain" };
inline constexpr const char* lVPan[]   = { "l0_vpan", "l1_vpan" };
inline constexpr const char* lVDecay[] = { "l0_vdecay", "l1_vdecay" };

} // namespace ParamID
