# Changelog

All notable changes to this project are documented here. Format: [Keep a Changelog](https://keepachangelog.com/), versioning: [SemVer](https://semver.org/).

## [Unreleased]
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
