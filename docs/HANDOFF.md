# Anal-Log (formerly Speakit): project handoff

Read this before working on Anal-Log. It records what was built, why, and what is still open, as of version 0.4.5 (2026-11-07). The product was renamed from Speakit to Anal-Log in 0.4.0; internal names (CMake target SPEAKIT, classes SpeakitProcessor/SpeakitEditor, repo yishayraziel/speakit) were kept.

## What it is

Anal-Log is Yishay Raziel's own speaker, phone and radio simulator plugin. It was inspired by the category Audio Ease Speakerphone 3 belongs to, but all DSP, IRs, samples, presets and UI are independent work.

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

- **IRs:** 101 total (23 speaker, 15 cover, 20 mic, 43 room), level-matched, stored as FLAC (5.6 MB). Built by `tools/prepare_irs.py <IR-Library> <MIT Audio> <BUT dir> <MicIRP dir>`, which writes `Source/IrManifest.h`; every row carries a license tag (`IrInfo::license`).
  - 46 from IR-Library by Conner (MIT, https://github.com/itsmusician/IR-Library).
  - 16 original "modeled" IRs generated from target curves (desk phone, payphone, walkie-talkie, megaphone, transistor radio, intercoms, TVs, modeled mics); the menu labels them "(modeled)". Real captures should replace them later.
  - 0.4.5: 26 rooms from the MIT IR Survey (CC BY 4.0; 32 kHz sources), 2 office positions from BUT ReverbDB room L207 (CC BY 4.0; 16 kHz source, so no content above 8 kHz), 11 vintage mics from MicIRP (CC BY-SA 4.0). Menu names describe the type, never the maker.
  - **Index stability:** choice parameters store an index, so new IRs are only ever appended to the end of a list (list `N` in the script). Menus and the < > arrows walk the list grouped by `group` (`irOrder()` / `irMenu()` in `PluginEditor.cpp`), so appended rows still land in the right submenu.
  - Raw downloads live on the owner's Mac in `~/Downloads/Anal-Log IRs/` (MIT `Audio.zip`, the 9.3 GB BUT `.tgz`, `MicIRP/`). Only room L207 of BUT was extracted: the Mac's shell kills processes after 3 minutes, and the other BUT rooms are 16 kHz and duplicate MIT's space types.
- **Sample Bay:** 60 sounds in 5 banks of 12 (STREET, INDOORS, LINE, MACHINES, MUSIC), Ogg Vorbis. Sources: CC0 recordings from noixer (from Freesound) and AtomCut packs, plus original synthesized line sounds (dial tones, DTMF, modem, hum, hiss). MIDI note 36 and up triggers pads. Built by `tools/prepare_samples.py`, which writes `Source/BayManifest.h`.
- **Presets:** 26 factory presets in `Source/Presets.h`. The display adds " *" when a preset has been edited.

## UI (0.4.0)

- Look adopted from the owner's earlier ChatGPT project: orange vintage-computer chassis (`Resources/skin/chassis.jpg`), green CRT, cream knobs and keycaps. Fonts: Barlow Condensed for panel labels, VT323 (OFL) for the screen.
- Canvas 1686 x 933, scaled as one; popup menus counter-scale.
- **Screen (left):** name, version, preset browser (< name * >, click name for list), selected module and its state, the signal chain in three rows (lit = on, outline = off, click to open), Drive transfer curve on the Drive page or a scrolling output picture elsewhere, IN/OUT meters.
- **Right panel:** the selected page. Module pages have an ON/OFF switch; controls dim when the module is off. Knobs show a numbered scale and value; choices with up to 4 options are stepped knobs, longer lists are green LCD boxes (IR and codec boxes have < > arrows).
- **Keys (bottom):** 15 modules in signal order plus IN/OUT, LFO, ENV, SAMPLES, MEMORY. The green LED on a module key means ON; clicking the LED toggles the module.
- **Bottom right:** DRY / WET macro and BYPASS (host bypass parameter, latency compensated, click-free crossfade).
- New in 0.4.0 DSP: Drive gained HPF, TONE, BIAS, ASYM and LINK (output compensates drive).

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

## Free IR sources (licenses)

Anal-Log is **free and non-commercial** (no ads, no upsell, no lead capture), so CC BY-NC sources are allowed. Tag every IR with its license in `tools/prepare_irs.py` so NC files can be removed if it is ever sold. Credit every source in `THIRD_PARTY_NOTICES.txt`.

- Done in 0.4.5: MIT IR Survey, BUT ReverbDB (L207 only), MicIRP. These sites are blocked from the cloud workspace and from the Mac's shell; the owner downloads by hand.
- Still open: Surrey microphone IRs, CC BY 4.0, 25 mics, 1.5 GB: zenodo.org/records/4633508. OpenAIR (York): check each file's license; CC BY and CC BY-NC are fine.
- Do not use: EchoThief (needs the author's permission), TONE3000 packs (no redistribution), Voxengo (only unaltered copies), IRs from Audio Ease's site.
- Device IRs (phones, radios, walkie-talkies, megaphones) are not available under free licenses; the plan is for the owner to record his own with a sweep.
- Releasing under CC BY-SA content: the MicIRP-derived files must stay CC BY-SA. Note this before any change of license or sale.

## Open items

1. Owner testing in Cubase of the new IRs (0.4.5): the grouped IR submenus, < > stepping, and older sessions reopening with the same IR. Also from 0.2.0: every switch and title toggle, IR and target pickers, Telecom switching on from OFF, the edited-preset star, readability at his window size.
2. An earlier screenshot showed the preset name SHOWER RADIO with settings that did not match it. Not yet confirmed whether he had edited it or it is a bug.
3. AU has never been tested; Windows has never been run and is unsigned.
4. Re-run ASan on 0.2.0; reduce worst-case CPU.
5. Confirm each noixer sound's license on its Freesound page.
6. Trademark check on the name "Anal-Log" (also: the name may be filtered by some stores or search engines) and a legal review before selling.
7. Decide whether the repo stays public (it exposes the code, skin, IRs and samples).
8. Replace the 16 modeled IRs with real captures (sweep method in the build spec).
9. Still simpler than the reference category: no user sample import, snapshots not automatable, Dry/Wet curves not saved per preset.
