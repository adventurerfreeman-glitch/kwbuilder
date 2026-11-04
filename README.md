# KW Builder V3 Knobs — 0.3.0

This is a COMPLETE REPLACEMENT SOURCE PROJECT. It contains no dropdown controls.
Ten rotary controls and two switches are created explicitly in the custom editor.
The plugin name and unique ID are new, to distinguish it from older installations.

## What you must see

Window title: KW BUILDER V3 - KNOBS + SWITCHES.
Main row: GAIN / LEVEL / TEXTURE, each a rotary knob with its current value.
Builder row 1: SOURCE VOICE / INPUT CAP C1 / Q1 BYPASS CAP / Q1 DRAIN R4.
Builder row 2: COUPLING C4 / HF SHUNT C3 / Q2 SOURCE R8.
Every one of those seven builder titles has a visible drawn rotary control and a
separate selected-value label. They step between fixed component options.
Right of row 2: OUTPUT PATH switch, INPUT LOAD switch.
Bottom: RESET STOCK / SCREAM-2 / SCREAM-3 / SCREAM-4 / active selection / peak meters.

No ComboBox exists in the source. The knobs are JUCE rotary Slider components,
not painted pictures. Each has a live SliderAttachment. Switches are live
ToggleButtons with ButtonAttachments. All controls are direct children of the
editor and all are given explicit nonzero bounds inside the 1120x800 window.
A local LookAndFeel draws contrasting knobs regardless of external theme colors.

Drag UP/DOWN on a knob or use its mouse wheel. Read the selected value directly
under it. You do not need to type a component value. Output path checked means
Scream; unchecked means Whisper. Input load checked means 2 MOhm, unchecked 1 MOhm.

## Fixed options

GAIN: continuous 0–10, default 5. LEVEL: -24 to +12 dB, default -6.
TEXTURE: continuous 0–10, default 5.
SOURCE VOICE: 1 Calm / 2 Sullen / 3 Moody / 4 Angry.
INPUT CAP C1: 47 / 100 / 220 nF, default 100.
Q1 BYPASS CAP: Follow Voice / Off / 100 nF / 1 uF / 22 uF.
Q1 DRAIN R4: 6.8 / 10 / 15 kOhm, default 10.
COUPLING C4: 10 / 22 / 47 nF, default 22.
HF SHUNT C3: 10 / 22 / 47 nF, default 22 (corrected source BOM value).
Q2 SOURCE R8: 680 Ohm / 1 kOhm / 1.5 kOhm, default 1 kOhm.
OUTPUT PATH: Whisper / Scream, default Scream.
INPUT LOAD: 1 / 2 MOhm, default 1.

Scream/2, /3, /4 set source voice, output Scream and bypass Follow Voice.
They preserve GAIN/LEVEL/TEXTURE and other part edits. Their rotary and switch
controls update through host parameter attachments. A status readout confirms
what is active. Follow Voice uses 100 nF / 1 uF / 22 uF for voices 2/3/4.
A manual bypass override can make 2/3/4 converge; buttons restore Follow Voice.
RESET STOCK resets all parameters to the defaults, with Scream/2 selected.

## Build and install

Use a clean repository/folder, or replace the old source project completely.
Do NOT keep old extra PluginProcessor.cpp/PluginEditor.cpp files in the build.
The included CMakeLists compiles ONLY Source/Plugin.cpp for the plugin.

GitHub:
1. Upload the CONTENTS of KWBuilderV3 at repo root, including hidden .github/.
2. Run Actions > Build KW Builder V3 Knobs (or push to trigger it).
3. After a successful run, download the correct platform artifact.
4. Install the COMPLETE VST3 bundle; do not extract only its internal library.
5. Rescan your DAW and instantiate **KW Builder V3 Knobs**, not V1/V2.

Typical VST3 locations:
Windows: C:\Program Files\Common Files\VST3.
macOS user: ~/Library/Audio/Plug-Ins/VST3.
Unsigned builds may require macOS security approval. Signing/notarization is not
provided. Do not run the downloaded standalone and assume that updates a DAW's
installed VST3; install the new VST3 separately.

Local build: CMake 3.22+, Git, C++17 toolchain and JUCE platform dependencies.

```sh
git clone --depth 1 https://github.com/juce-framework/JUCE.git JUCE
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Artifacts: build/KWBuilderV3_artefacts/ (Release subfolder in supplied CI).
VST3 and Standalone targets included. The workflow downloads JUCE's default
branch; record the logged commit and pin it once a build is known good.
JUCE and related SDK licenses are separate. Review them before redistribution.

## Acceptance check — before judging the sound

- Title says V3 KNOBS + SWITCHES.
- Count 10 visible knobs, NOT three knobs with seven empty title areas.
- Each builder knob changes its selected-value label when dragged.
- Output Path switch changes Whisper/Scream in the active-status line.
- Scream/3 visibly moves Source Voice to 3, Output to Scream, Bypass to Follow Voice.
- GAIN/LEVEL/TEXTURE do not move when changing voices.
- Save/reopen a DAW project: values should restore. This still needs host testing.

If labels remain without knobs, a screenshot showing the full V3 title, the
specific build artifact, OS and DAW is needed. Do not assume success merely from
this source package existing.

## Audio and limitations

This retains V2's stronger behavioral drive: GAIN increases both nonlinear stage
inputs plus interstage drive. TEXTURE adjusts drive and asymmetry; it also changes
level. LEVEL follows the core. Default GAIN 5/10 reflects your recorded reference
context. Scream voices use distinct bypass frequency-weighting parameters.

No amp or cab is baked in: dry DI -> plugin -> amp simulator -> cabinet is an
appropriate audition setup. Start monitoring quietly. An extreme output setting
can activate full-scale protection; protection is not part of the modeled pedal.

The DSP is two biased tanh stages, RC-inspired coupling and source-bypass
weighting. It is NOT a JFET circuit solver, SPICE/WDF model or matched capture.
Drain/source controls are bounded behavioral proxies, not solved bias conditions.
Several original loading networks are simplified and C3 uses an approximate
pot-dependent impedance. Input load is not a full pickup/cable model. No extra
LEAN EQ or new distortion circuit is added. Do not use this to predict hardware
component substitutions with engineering accuracy.

The full core is processed at 4x rate with JUCE oversampling; latency is reported.
Main knobs are smoothed. Stepped controls can still click during large changes;
click-free branch morphing is not claimed. Stereo channels maintain independent
state. Audio processing uses fixed chunk capacity. Host output is finite-checked
and limited at full scale. Keep output conservative when feeding an amp model.

Reference recording context: Scream/2 then /3 then /4, all GAIN 5/10. Not fitted
to the recording; no matching dry DI or exact segmentation is available. No
recording is redistributed and no perceptual-match claim is made.

## Validation actually performed

The independent DSP core compiled here with g++ -std=c++17 -O2 -Wall -Wextra
-pedantic and passed: all 12 parameter waveform-difference tests, distinct
Scream/2 /3 /4, 72 finite/bounded sample-rate/voice/path/setting cases, invalid-input
recovery and silence. See tests/results.csv.

The FULL JUCE plugin and custom editor have NOT been compiled/rendered/run in a
DAW here. Source audit is not runtime UI verification. GitHub Actions runs core
tests after compiling, but requires an actual successful build. No claim is made
that your specific host displays the interface until you load that build.
