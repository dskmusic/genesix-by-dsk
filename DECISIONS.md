# Design decisions

Decisions made where the brief was ambiguous; the most robust option was chosen.

| # | Topic | Decision | Why |
|---|-------|----------|-----|
| 1 | License | **GPLv3** | JUCE 8 is GPLv3/commercial; freeware with public source and no JUCE commercial licence must be GPLv3. VST3 SDK is MIT. |
| 2 | JUCE version | **8.0.17-security** (latest 8.x tag) | Brief asks for JUCE 8; JUCE 9 exists but is out of scope. Pinned tag, fetched with FetchContent or taken from `third_party/JUCE` offline. |
| 3 | Dependency manager | Plain `FetchContent` (no CPM) | Only one dependency (JUCE); CPM adds nothing. |
| 4 | Repo visibility | Public from the first push | Windows Actions minutes are free and unmetered on public repos; the `.gitignore` audit ran before the first commit. |
| 5 | `build_local.sh/.bat` | **Committed** | They are generic (no personal paths); deps dir overridable with `GX_DEPS`. Only `BUILD_LOCAL.md` is ignored. |
| 6 | `prompt inicial.txt` | Ignored | Personal working file. |
| 7 | Windows deps path | `%USERPROFILE%\.gx-deps` | Project path contains spaces and is deep; JUCE object paths would hit MAX_PATH. |
| 8 | `COPY_PLUGIN_AFTER_BUILD` | FALSE | Copying to Common Files\VST3 needs admin rights. |
| 9 | Plugin codes | Manufacturer `Dskm`, plugin `Gnsx` | Frozen forever (DAW session compatibility). |
| 10 | Shared code | One INTERFACE source set compiled into the plugin and into `GenesixTool` | The tool runs unit tests, offline renders, preset validation, factory bank generation, SFZ export and screenshots with the exact same engine. |
| 11 | Commit e-mail | Repo-local GitHub noreply address | The repo is public; keeps the personal e-mail out of commit metadata. |
| 12 | macOS | Universal binary (arm64+x86_64), VST3 + AU + Standalone, min macOS 11, ad-hoc signed, unsigned `.pkg` | No Apple Developer ID available: binaries are not notarized (users must allow them in Gatekeeper). Priority: Windows + macOS; Linux kept while it costs nothing. |
| 13 | Preset index | JSON (`index.json` + `library.json` overlay), no SQLite | Thousands of entries index in milliseconds; avoids a native DB dependency on 3 platforms. |
| 14 | FX "10 slots" | 12 effects, each in its own reorderable slot | Every effect keeps stable, automatable parameters; order is part of the preset. |
| 15 | Modulation rate | Control rate (16 samples) with per-sample amp interpolation | Big CPU saving; 3 kHz modulation rate is inaudible for LFO/env modulation. |
| 16 | UI language | Spanish UI (section names exactly as in the brief); AI messages selectable ES/EN | The brief defines the sections in Spanish; parameter names stay in English for DAW automation lists. |
| 17 | Level control | Sustained sounds -> target RMS per category; decaying sounds -> peak normalisation; always peak <= -1 dBFS; master range -48..+24 dB; oscillator levels raised when master range is not enough | Percussive sounds cannot reach an RMS target without clipping. |
| 18 | Generator validation | Offline render, reject: NaN/Inf, silence, DC > 0.02, missing sustain (sustained categories), non-decaying loud tail, spectral flatness > 0.45 or centroid > 7.5 kHz (tonal categories), centroid < 40 Hz; regenerate up to 24 times | Musical sanity without dice-roll presets. |
| 19 | Factory bank | Generated deterministically by `GenesixTool factory` (20 per category, near-duplicate rejection per subcategory, unique names), committed as `resources/factory.genesixbank` and embedded; CI re-validates every preset | Reproducible builds; DSP regressions show up as test failures. |
| 20 | Undo/redo | Snapshot polling (params + patterns) every 300 ms when no mouse button is down | Catches every edit (knobs, grids, drawn LFOs, matrix) with one mechanism. |
| 21 | SFZ | Timbre (filters, FX, unison) baked into samples; SFZ filter left open; loops only for sustained, non-rhythmic sounds; loop length = whole periods + baked crossfade | SFZ cannot reproduce the engine; this gives the closest static result without clicks. |
| 22 | Linux | Kept (CI green on Ubuntu 24.04) | User asked to drop it only if it caused trouble; it did not. |
| 23 | Screenshots | `GenesixTool screenshot` renders the real editor offscreen (Windows) and saves PNGs in `docs/screenshots` | Real UI, reproducible, no manual capture. |
| 24 | Installers | Inno Setup `.exe` (Windows), unsigned `pkgbuild` `.pkg` (macOS) | No code-signing identities available; documented in README. |
| 25 | Logo | Vector mark drawn in code (hexagon + double helix "genesis") + "Genesix" with gradient, "by DSK" small; click opens dskmusic.com | No external font/asset licences needed; scales perfectly. |
