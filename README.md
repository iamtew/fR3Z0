<img src="assets/logo.png" alt="fR3Z0" width="160">

# fR3Z0

**fR3Z0** is a resonator effect. Tune a bank of bandpass voices, set decay and mix, drag peaks on the graph, and modulate with two LFOs. Every control is its own parameter.

VST3. Stupid Audio. [MIT](LICENSE). Site: [docs/](docs/index.html). Manual: [docs/docs.html](docs/docs.html).

Standalone is built for debug and `just docs-screenshots` only. Dist ships the VST3. An effect needs a host; the standalone has no DAW input (noise still rings).

The site backdrop is `docs/fx.js`, a PixiJS scene kept in sync with the other Stupid Audio sites. Add `?fx` to any page to preview it full screen with the page chrome hidden; `?fx=false` keeps the normal page.

## Release

Windows, Linux, and macOS files come from GitHub Actions (`.github/workflows/dist.yml`). Run it from the Actions tab for artifacts only, or push a numeric tag such as `0.0.1` (no `v` prefix) to build and publish a GitHub Release with the files attached. `<version>` is the `version` in `fR3Z0.jucer`.

The `dist` artifact holds:

- `fR3Z0-<version>-win-x64-vst3.zip`
- `fR3Z0-<version>-win-x64-installer.msi`
- `fR3Z0-<version>-linux-x64-vst3.zip`
- `fR3Z0-<version>-mac-universal-vst3.zip`

The MSI installs the VST3 into `Common Files\VST3` by default; the folder can be changed.

The Mac zip is ad-hoc signed, not notarized. Gatekeeper blocks it until you right-click Open, or run `xattr -dr com.apple.quarantine` on the `.vst3`. The Linux build uses the system ALSA, FreeType, fontconfig, and X11 libraries.

`just build-dist` on Windows writes the VST3 zip and the MSI. Building the MSI needs the .NET 8 SDK.

`just docs-screenshots` overwrites `docs/Screenshot1.png` and `docs/Screenshot2.png`. Needs a debug standalone (`just build-debug`) and an interactive desktop.
