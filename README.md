# KW Builder V4 — PRE LOW CUT / POST TILT

SOURCE PACKAGE, not a precompiled VST3. New plugin identity KwB4; product name
KW Builder V4 PreCut PostTilt. This is an updated behavioral prototype, not an
exact circuit model or fitted reference capture.

## Corrected signal path

INPUT -> PRE LOW CUT -> original-inspired input coupling -> Q1 nonlinear stage
-> interstage GAIN / C3 -> Q2 nonlinear stage -> selected Whisper/Scream output
-> POST TILT EQ -> LEVEL -> output protection -> AMP.

Both paths receive the pre low cut. Both selected outputs receive the post tilt.
Post tilt is NOT fed back into either saturation stage. LEVEL is after the EQ.
The added low cut does not replace the original input/coupling networks.
No cab or amplifier is baked in, and there is no PRE/POST routing toggle.

## Added controls (exploratory ranges)

PRE LOW CUT: OFF / 40 / 65 / 90 / 120 / 180 Hz. Default 65 Hz. A first-order
high-pass, approximately 6 dB/octave below its corner. It reduces low-frequency
energy BEFORE either nonlinear stage. This can reduce bass-driven saturation;
we do not compensate by automatically adding more internal drive.

POST TILT EQ: -3 to +3 dB. Default 0 dB (flat). Fixed provisional pivot 800 Hz.
Positive tilt: lows approach -T dB, highs +T dB, pivot approximately 0 dB.
Thus +3 gives about 6 dB end-to-end difference, NOT +3 dB on everything.
Negative reverses that balance. The filter is an RBJ low shelf of -2*T dB plus
+T dB compensation, shelf slope S=1. Source: W3C Audio EQ Cookbook:
https://www.w3.org/TR/audio-eq-cookbook/ . Coefficients implemented in Source/DSP.h.
It is a broad tilt, not the previous experimental asymmetric LEAN/MIDS system.
No new MIDS, pivot-mode or Q controls are invented in this version.

CUT OFF / TILT FLAT button disables both added treatments (leaves all other knobs
unchanged) to compare the baseline. RESET V4 restores 65 Hz / flat tilt, component
reference choices, Scream/2, GAIN 5/10, TEXTURE 5/10, LEVEL -6 dB.
These are starting values selected for this prototype, not measured pedal values.

## Visible controls — 12 knobs and 2 switches

Top: GAIN / LEVEL / TEXTURE / PRE LOW CUT / POST TILT EQ.
Builder knobs: SOURCE VOICE / INPUT CAP C1 / Q1 BYPASS CAP / Q1 DRAIN R4 /
COUPLING C4 / HF SHUNT C3 / Q2 SOURCE R8.
Switches: OUTPUT PATH (checked=Scream) and INPUT LOAD (checked=2 MOhm).
Each rotary is a real attached Slider, with an explicit position and value label.
No dropdowns. Part/low-cut selectors snap to integer choice positions.

Other options:
SOURCE VOICE: 1 Calm / 2 Sullen / 3 Moody / 4 Angry.
C1: 47 / 100 / 220 nF. Bypass: Follow Voice / Off / 100 nF / 1 uF / 22 uF.
R4: 6.8 / 10 / 15 kOhm. C4/C3: 10 / 22 / 47 nF. R8: 680 / 1000 / 1500 Ohm.
Input load: 1 / 2 MOhm (simplified load, not pickup/cable model).
C3 reference = 22 nF as shown in the corrected perfboard BOM.

Scream/2 /3 /4 buttons explicitly select source voice, Scream path and Follow
Voice bypass. They preserve GAIN, LEVEL, TEXTURE, LOW CUT, TILT and other parts.
Follow Voice maps 2/3/4 to 100 nF / 1 uF / 22 uF. Their waveforms are distinct in
the supplied tests. If you manually override bypass, voices sharing the same
source resistance may converge. The buttons restore Follow Voice.

## GAIN / saturation check

GAIN increases drive into BOTH nonlinear stages plus interstage drive. It is not
just output gain. LEVEL is independent. TEXTURE affects nonlinear intensity and
asymmetry, and also level; no automatic loudness compensation is claimed.
We retained the stronger V2/V3 drive law rather than blindly increasing it again.

Tested the independent core at GAIN 0 / 2.5 / 5 / 7.5 / 10 for Scream voices
2/3/4 with input peaks -36 / -24 / -12 dBFS: 45 saturation cases.
All use a 110 Hz sine, TEXTURE 5/10, low cut 65 Hz, tilt flat, LEVEL -6 dB.
The core runs at 192 kHz. Measurements use harmonics 2–12 after settling.
See tests/gain_saturation_report.csv for every measurement.

At -24 dBFS input peak and GAIN 5/10:
Voice 2 THD: 12.4302%; Voice 3: 19.5519%; Voice 4: 27.4890%.
Output protection engaged for 0% of samples in all 45 test cases. Thus the
reported distortion is from the behavioral stages, not a full-scale output clamp.
All nine gain sweeps increased in measured THD at every tested step.

These are synthetic sine-test results, NOT measured THD of your Kasha pedal,
not a reference fit and not proof of musically appropriate saturation. Input
calibration matters: gain midpoint is not a universal distortion amount.
The test shows drive is functional and substantial at the specified inputs.
An actual matched dry/wet recording is still needed for reference calibration.
Your wet reference was described as Scream/2 then /3 then /4, GAIN 5/10.
Do not run that already-processed reference through this plugin as though it
were dry input when assessing a match.

## Other checks performed here

- Independent core compiled with g++ -std=c++17 -O2 -Wall -Wextra -pedantic;
  no compiler diagnostics, all test assertions passed.
- All fourteen parameter changes produced waveform differences in the test.
- Scream/2 /3 /4 are distinct.
- At fixed input and other controls, +3 dB post tilt left BOTH pre-nonlinearity
  stage drive indices unchanged while changing the output waveform.
- Stronger pre low cut reduced stage-1 drive on a low-note sine.
- Tilt measured approximately 0 dB at 800 Hz, -3 dB at 40 Hz and +3 dB at 8 kHz
  for the +3 setting; zero tilt was unity in the filter test.
- 72 finite/bounded stability cases across 44.1/48/96 kHz host-rate equivalents,
  voice/path/settings; invalid-input recovery and silence checks.

The FULL JUCE plugin/editor/oversampling wrapper has NOT been compiled or run in
a DAW here. Independent core tests and source checks do not replace host testing.
The supplied CI workflow builds the plugin and runs tests when actually executed.

## Build using GitHub

Use a clean folder/repository or replace old source completely. Upload the
CONTENTS of KWBuilderV4 to repository root, including hidden .github/.
Run Actions > Build KW Builder V4. After a successful build download the correct
Windows/macOS artifact, install the COMPLETE VST3 bundle, rescan the DAW and load
**KW Builder V4 PreCut PostTilt**. Old plugin instances do not update themselves.

Expected window title: KW BUILDER V4 - PRE LOW CUT / POST TILT.
Count 12 knobs, 2 switches. Confirm changing Pre Low Cut and Post Tilt moves their
value labels. Confirm the Scream buttons update the voice knob and path switch.

Typical VST3 locations:
Windows: C:\Program Files\Common Files\VST3.
macOS user: ~/Library/Audio/Plug-Ins/VST3.
Unsigned builds may require security approval. Signing/notarization not included.
The standalone requires audio device configuration and is NOT installed into
your DAW merely by launching it. Use headphones to avoid live feedback.

Local build (CMake 3.22+, Git, C++17 compiler, JUCE platform dependencies):

```sh
git clone --depth 1 https://github.com/juce-framework/JUCE.git JUCE
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Artifacts under build/KWBuilderV4_artefacts/ (Release in supplied CI).
VST3 and Standalone, not VST2. CI fetches JUCE's current default branch; pin the
printed commit after a successful validated build for reproducibility.

## Modeling / monitoring limitations

The nonlinear stages are biased tanh functions, NOT physical JFET equations.
R4/R8 change behavioral drive proxies, not solved DC operating points. Coupling
loads and pot impedance are approximated. Original device variability and all
loading interactions are not represented. Do not treat this as a component-
accurate hardware-building predictor. Keep the original schematic as reference.

JUCE 4x oversampling wraps the full core; latency is reported. Stereo channels
are independent; mono is supported. Audio loops use fixed chunk sizes and do
not deliberately allocate or perform file IO. GAIN/LEVEL/TEXTURE/TILT targets
are smoothed. Stepped component/low-cut changes can still click; stop playback
for critical large edits. Extreme LEVEL/tilt can engage protection; a visible
OUTPUT GUARD warning tells you to lower LEVEL. This is protection, not pedal tone.
No automatic output normalization hides the gain changes.

The UI meters are approximate digital peak holds, not hardware meters. Use the
plugin before an amp simulator to assess the intended amp-facing behavior.
Always start monitoring quietly. Distribution signing and external license
compliance are separate responsibilities; JUCE licenses are not supplied here.
