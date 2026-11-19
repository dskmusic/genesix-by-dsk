# Changelog

All notable changes to this project are documented here. Format: [Keep a Changelog](https://keepachangelog.com/), versioning: [SemVer](https://semver.org/).

## [Unreleased]
### Added
- Generator v2 (sound design): every request now builds several valid candidates and keeps the most interesting one (category-aware score for movement, evolution, stereo width and richness; chaos and noise are penalised).
- Layering with roles (air layer an octave up, clean sub weight, spectral-mutation motion layer), Genesix signature features where they shine (Shimmer, Granular Smear, formant and comb colour), stereo tremolo/phaser on keys, slow "bloom" envelope and drift LFO on big sustained sounds.
- Musically meaningful macros (Brightness, Timbre, Space, Character with several targets each) and a mod-wheel "open up" morph on every tonal sound; Weirdness now adds controlled originality (harmonic FM colour, key-tracked timbre, velocity formants).
- 7 new archetypes: Bass Growl, Lead Anthem, Lead Vocal, Pad Cinematic, Strings Hybrid, Brass Trailer, Ambient Cathedral.
- `GenesixTool bank-report` (structural + audio analysis of a preset bank) and `GenesixTool listen-pack` (fixed loudness-matched A/B listening pack).
- `GenesixTool video <dir>`: scripted 1080p30 product demo rendered from the real editor and engine (frames + audio, mux with ffmpeg).
### Fixed
- Factory preset notes were shown in Spanish with the English interface: they are now stored in English and shown in Spanish only when the interface is Spanish.

## [1.1.0] - 2026-11-19
### Added
- Full English/Spanish interface (all labels, menus, tooltips, dialogs and SFZ warnings). First run follows the system language; switch it in Settings > Idioma / Language (the whole interface is rebuilt instantly, keeping the current page and generator settings).
- Application icon for the Standalone `.exe`/`.app`, the Windows installer and its uninstaller entry (`branding/`, regenerate with `GenesixTool icon branding`).
### Changed
- The generator history (◀ ▶) now keeps only the current session: it lives in memory, starts empty each time the plugin loads and is no longer written to disk (the old `history.json` is deleted).
- README screenshots retaken with the English interface (`GenesixTool screenshot <dir> [en|es]`).

## [1.0.0] - 2026-11-19
### Added
- **GENERADOR**: rule-based smart preset generator (11 categories, 50+ archetypes, 5 character sliders, section locks, mutate, blend), offline validation with automatic level control, animated "AI" console (ES/EN, 54+ messages per category), parameter resolve/assemble animation, live oscilloscope/spectrum/VU/LED matrix/circuit bus, persistent browser-like history (◀ ▶, Alt+←/→).
- **PRESETS**: Omnisphere-style manager (category/sub-category tree, character tags, type/author/folder filters, instant search, sorting, stars, favourites, use count, preview on select, keyboard navigation, save/save as/rename/duplicate/delete/move/edit metadata, user folders, import/export of presets and banks).
- **201 factory presets** (all generated, validated and de-duplicated; arp, trance gate and step-sequencer presets included).
- **EDITOR**: OSC (waveform views, wavetable editor/importer), FILTROS (response view), MOD (envelope views, drawable LFOs, macros, drag-and-drop matrix), ARP/GATE/SEQ (step lanes), FX (reorderable chain); undo/redo, A/B, section copy/paste, MIDI learn.
- Simple sampler: load a WAV/AIFF/FLAC into the Noise/Sample oscillator (embedded in presets).
- **EXPORTAR**: `.genesix` export/import and SFZ multisample export (velocity layers, crossfaded loops, batch, cancellable progress, warnings).
- **AJUSTES**: UI scale 50–200 %, animation level, AI messages on/off and language, history limit, preview on select, MIDI-learn reset.
- FL-Studio-style LookAndFeel, vector logo linking to dskmusic.com, About box.
- macOS universal build (VST3 + AU + Standalone, pkg installer), Windows installer, SHA256 checksums.

## [0.1.0] - development
- Preset system: versioned `.genesix` JSON (params, mod matrix by id, patterns, embedded wavetables/samples, metadata) with migration hook; plugin state uses the same format.
- Preset library: factory bank + user folder (Documents/DSK Music/Genesix), cached JSON index, favorites/ratings/use counts overlay, instant multi-field search, category/subcategory/tags/type/author/folder filters, sorting, rename/duplicate/delete/move, user folders, import/export of presets and `.genesixbank` banks.
- Synth engine: 32-voice allocator (poly/mono/legato, glide, sustain, MPE), unison up to 8 (detune/spread/blend).
- Oscillators: 24 procedural band-limited mip-mapped wavetables + importer, PolyBLEP analog with self-sync/PWM/morph, 4-op FM (8 algorithms, feedback), noise (white/pink/brown/crackle/digital) + sampler, and the original **Spectral Mutation** oscillator (evolving harmonic genome).
- 2 filters (serial/parallel): SVF LP/HP/BP/Notch 12/24, nonlinear ladder, comb +/-, formant; drive, key-tracking, env amount.
- 4 DAHDSR envelopes with curves, 4 LFOs (8 shapes incl. drawable, host sync, retrigger, fade-in, phase), 8 macros, 16-slot modulation matrix (24 sources, ~140 destinations incl. virtual pitch/volume/pan).
- Rhythm: arpeggiator (8 modes, octaves, swing, per-step gate/velocity/ratchet/probability/transpose, latch), trance gate, modulation step sequencer, chord/strum, humanize.
- 12 reorderable FX: distortion (4x oversampled), chorus, flanger, phaser, ping-pong delay, FDN reverb, compressor, 4-band EQ, bitcrusher, widener, **Shimmer** and **Granular Smear**.
- Unit tests (1165 checks): parameter table, oscillators, filters, FX, determinism, sample-rate/block-size changes, state round-trip.
- Project skeleton: CMake + JUCE 8, VST3 + Standalone, CI (Windows/Linux), pluginval, release packaging.
