# fR3Z0 – Cursor Rules

Spec: [docs/SPEC.md](docs/SPEC.md). Notes: [docs/Resonator_VST3_Core_Intermediate_LFOs.md](docs/Resonator_VST3_Core_Intermediate_LFOs.md), [docs/Resonator_VST3_Visualizers_Graphics.md](docs/Resonator_VST3_Visualizers_Graphics.md). Advanced later: [docs/Resonator_VST3_Advanced_Future.md](docs/Resonator_VST3_Advanced_Future.md).

Resonator effect. Many host-automatable parameters (globals, eight voices, two LFOs). The graph edits the same parameters; it does not hide them. VST3. Part of Stupid Audio; sister of fN0B.

Standalone stays in the jucer for debug and `just docs-screenshots`. Do not ship it. Dist is VST3 only.

## Stack
- JUCE (Projucer workflow only – no CMake)
- C++17, real-time safe: no allocations, no locks, no system calls in processBlock
- APVTS for all parameters
- Just + justfile
- JUCE from `%USERPROFILE%\JUCE`
- `assets/logo.png` is the Affinity export. Source of truth. Never overwrite. `just icon` / `just docs-logo` derive the rest.

## Layout
src/
 PluginProcessor.h/.cpp
 PluginEditor.h/.cpp
 dsp/Units.h Bank.h Presets.h
 parameters/ParameterIDs.h ParameterLayout.h
 ui/Theme.h VersionCompare.h
tests/fr3z0_check.cpp
docs/
justfile
fR3Z0.jucer

## Version
- Jucer `version=` is what the binary and About dialog show.
- Git tags must match that string (`0.0.1`).
- `just build-dist` names the zip and MSI from the jucer version.

## Colors
- #E038E9 Hot Magenta (primary, knobs)
- #8A1AC6 Electric Purple (primary)
- #4B0082 Deep Violet (wells, panels)
- #2FC1EE Cyan (ticks, selection)
- #3143D4 Electric Blue (secondary)
- #F0F8FF Near White (text)
- #000000 Pure Black (background)

## Do NOT
- Introduce CMake
- Allocate on the audio thread
- Collapse the UI into one macro knob
- Copy commercial plugin code
