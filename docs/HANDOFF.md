# Speakit: project handoff

Read this before working on Speakit. It records what was built, why, and what is still open, as of version 0.2.0 (commit a2a119a, 2026-11-06).

## What it is

Speakit is Yishay Raziel's own speaker, phone and radio simulator plugin. It was inspired by the category Audio Ease Speakerphone 3 belongs to, but all DSP, IRs, samples, presets and UI are independent work.

- Formats: VST3, AU, Standalone. macOS (signed and notarized) and Windows (unsigned).
- Framework: JUCE 8.0.4, C++17, CMake.
- Repo: https://github.com/yishayraziel/speakit (public, branch `main`).
- The owner wants every plugin to show a small version number in its UI. It sits top right, under nothing else but "by Yishay Raziel".

## Hard rules (legal)

- Never reverse engineer Speakerphone: no disassembly, decompiling, or tools such as rea on its binary. Its EULA clause 2a forbids it without Audio Ease's written permission, and owning a legal license does not change that.
- Never sample, re-record or reuse Speakerphone's IRs (EULA 2b), or use them for AI training (2c).
- No "Speakerphone" or "Audio Ease" in the product name, marketing or tags. Describe IRs generically, not by device brand.
- Allowed: listening comparisons by the owner, and building from public descriptions.
- Ship `THIRD_PARTY_NOTICES.txt` with every release (it is bundled in the binary too).

## Signal chain (fixed order, in `PluginProcessor.cpp`, `chain()`)

Input gain, then Sample Bay PRE bus, then Speaker if set to PRE, then Gate, Comp, EQ, Drive, Telecom, Crush, Mod, Gramophone, Radio Tuning, then Speaker (POST, the default), Cover, Mic, Leslie, Delay, Room, then Sample Bay POST bus, output gain, Out Limiter.

- **Dry/Wet macro:** one control mapped to a smoothstep curve per module (`tgt()` in `chain()`), not a plain crossfade. Each module's mix is smoothed per sample (fixes clicks).
- **Latency:** fixed and reported at all times: telecom pipe + drive oversampling + limiter lookahead, about 34 ms at 48 kHz. Bypass delays dry by the same amount.
- **Control rate:** parameters are read per 32-sample slot; audio is processed in 256-sample chunks.
- **Safety net:** any non-finite output is zeroed and the module states reset; `guardHits` counts it, and QA fails if it ever fires.

## Modules

| Module | Main control | Notes |
|---|---|---|
| Gate | Threshold | Range, release |
| Comp | Amount | Ratio, attack, release, gain |
| EQ | Narrow (telephone band macro) | HPF, LPF, 2 shelves, 2 peaks |
| Drive | Pre gain | 6 types, curve, pre EQ, 4x oversampled, DC blocker |
| Telecom | Damage (bursty frame loss) | Own on/off switch; real codec round trip at 8 kHz |
| Crush | Amount | Bits plus sample-and-hold, no anti-alias by design |
| Mod | | Tremolo, chorus, phaser, flanger, vibrato; tempo sync |
| Gramophone | | Wow, warp, ticks, gain |
| Radio Tuning | | Offset, whistle, AM/FM |
| Speaker, Cover, Mic, Room | IR picker plus Level or Mix | JUCE convolution, IRs load off the audio thread |
| Leslie | | Brake/slow/fast, inertia, width |
| Delay | | Stereo, sync, cross feed, LP/HP |
| Modulation | | 4 LFOs, 2 envelope followers, targets: Wet, Drive, Line, Crush, EQ, Mic, Cover, Room, Output |

**Codecs** (`Telecom.h`): G.711 u-law and A-law and IMA ADPCM (own code), GSM 06.10 (libgsm), Speex 15K, 8K and 2K, LPC Robot and LPC Whisper (own LPC vocoder). All share one fixed delay.

## Content

- **IRs:** 62 total (23 speaker, 15 cover, 9 mic, 15 room), level-matched, stored as FLAC. 46 come from IR-Library by Conner (MIT, https://github.com/itsmusician/IR-Library). 16 are original "modeled" IRs generated from target curves for devices the library lacks (desk phone, payphone, walkie-talkie, megaphone, transistor radio, intercoms, TVs, modeled mics); the menu labels them "(modeled)". Real captures should replace them later. Built by `tools/prepare_irs.py`, which writes `Source/IrManifest.h`.
- **Sample Bay:** 60 sounds in 5 banks of 12 (STREET, INDOORS, LINE, MACHINES, MUSIC), Ogg Vorbis. Sources: CC0 recordings from noixer (from Freesound) and AtomCut packs, plus original synthesized line sounds (dial tones, DTMF, modem, hum, hiss). MIDI note 36 and up triggers pads. Built by `tools/prepare_samples.py`, which writes `Source/BayManifest.h`.
- **Presets:** 26 factory presets in `Source/Presets.h`. The display adds " *" when a preset has been edited.

## UI (0.2.0)

- Skin: textures and parts cut from the owner's approved ChatGPT render by `tools/make_skin.py` and `tools/cut_parts.py`. Fonts: Barlow Condensed (OFL). LCDs use a built-in 5x7 dot font.
- **MAIN page:** full-width tabs (MAIN, EDIT, FX, SAMPLES); three numbered rows that read as the signal path: 1 PREPARE (Input, Gate, Comp, EQ), 2 TRANSMIT (Drive, Telecom, Crush, FX summary), 3 PLAYBACK (Speaker, Cover, Mic, Room). Each card has ON/OFF with an amber LED, dims when off, and toggles by its switch or title. Right column: Dry/Wet (whole chain), IN and OUT meters with a target zone, Output, Sample Bay status. Footer: LFO 1, LFO 2, ENV with a "TO:" target picker; speaker grille at bottom right.
- EDIT, FX and SAMPLES pages hold all deep controls, 10 snapshots, and the 60 pads.
- The editor is drawn on a 1932 px canvas and scaled; popup menus counter-scale so their text stays readable.

## Build, sign, release

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
cmake --build build --config Release --parallel
```

- Pushing to `main` runs `.github/workflows/build.yml`: Mac builds a DMG with a signed installer for VST3, AU and the app, notarized and stapled; Windows builds an Inno Setup installer. Results are downloaded from the run's Artifacts.
- Signing secrets were added once with `installer/setup_mac_signing.sh`. Never put certificates or passwords in the repo or in chat.
- Version lives in `CMakeLists.txt` (`project(SPEAKIT VERSION x.y.z)`); bump it for each build that goes to the owner.
- Chrome may block the zip as a "dangerous download"; GitHub CLI download works.

## QA

Build tests with `-DSK_TESTS=ON`. `tests/qa.cpp` (`sk_qa <voice.wav> [groups]`) covers sample rates 44.1 to 192 kHz, block sizes 1 to 8192, latency and codec alignment, every IR and sample, presets, 300 random-setting fuzz runs, every parameter at min and max, tails, null tests, clicks, state and snapshots, transport and MIDI, real-time allocation, multi-instance, threads and a 5-minute soak. Also run pluginval at strictness 10 and an AddressSanitizer build (`build_asan`).

0.2.0 results: 114 of 115 passed. The only failure is CPU with every module on at 48 kHz, block 64: about 65% of one cloud core (target 60%); normal use is about 3 to 4%. pluginval passed. ASan was clean on 0.1.4 but was not re-run on 0.2.0.

Bugs found and fixed by QA: out-of-bounds delay-line read (caused intermittent NaNs), clicks on output, codec swaps and IR module toggles, tiny popup menus, bypass latency, Speex delay mismatch, offline bounce before IRs were ready, host re-selecting a program wiping user edits, MIDI trigger allocating on the audio thread.

## Open items

1. Owner testing of 0.2.0 in Cubase: every switch and title toggle, IR and target pickers, Telecom switching on from OFF, the edited-preset star, readability at his window size.
2. An earlier screenshot showed the preset name SHOWER RADIO with settings that did not match it. Not yet confirmed whether he had edited it or it is a bug.
3. AU has never been tested; Windows has never been run and is unsigned.
4. Re-run ASan on 0.2.0; reduce worst-case CPU.
5. Confirm each noixer sound's license on its Freesound page.
6. Trademark check on the name "Speakit" and a legal review before selling.
7. Decide whether the repo stays public (it exposes the code, skin, IRs and samples).
8. Replace the 16 modeled IRs with real captures (sweep method in the build spec).
9. Still simpler than the reference category: no drive curve display, no user sample import, snapshots not automatable, Dry/Wet curves not saved per preset.
