# K/W Builder Prototype — source package 0.1.0

An experimental JUCE VST3 and standalone guitar-effect builder. This is actual
processor/editor source, not an HTML mockup. It implements a simplified behavioral
signal model and fixed-option builder controls.

## Honesty / current validation

- NOT a component-faithful circuit solver, SPICE/WDF model, neural capture, or
  reference-matched Kasha/Whisper emulation.
- The DSP core was compiled with g++ and smoke-tested in this environment.
  `tests/smoke_results.csv` records 72 finite-output cases across sample rates,
  voices, paths and component-choice combinations. This tests numerical behavior,
  not sonic fidelity, complete automation, exhaustive combinations or aliasing.
- The complete JUCE plugin has NOT been compiled or loaded in a DAW here. The
  supplied GitHub Actions workflow is a build route, not a claim of a passed build.
- No test of the JUCE editor, oversampling, plugin state or host integration was
  possible here. Build and validate before relying on this in a session.

## Get a plugin using GitHub

1. Unzip this source package. Put its contents at the root of a new GitHub repo:
   CMakeLists.txt, Source/, tests/, .github/, README.md and the other included files.
   Ensure the hidden `.github/workflows/build.yml` is uploaded.
2. Enable GitHub Actions if needed. Open Actions > Build KW Builder and run it,
   or push the files to trigger a run.
3. On a successful run, download the Windows or macOS artifact. Keep the complete
   VST3 bundle/folder, not just a file from inside it.
4. Install the VST3 into your platform's VST3 folder and rescan in your DAW.
   Windows: typically C:\Program Files\Common Files\VST3 (permission may be needed).
   macOS user: ~/Library/Audio/Plug-Ins/VST3 . Unsigned builds may require local
   security approval; distribution signing/notarization is not included.
5. Start with low monitor volume and insert it on a DI guitar track. Follow it
   with an amp simulator if that is your intended listening context.

The workflow downloads the default JUCE branch at build time. For reproducible
builds, record the successful JUCE commit and replace the clone step with a
checkout of that exact commit. Review dependency licenses before distribution.
The workflow requires internet, an available Actions runner and sufficient quota.

## Build locally

Install CMake 3.22+, Git and a C++17 toolchain. On Windows use Visual Studio C++
Desktop tools; on macOS use Xcode tools. Linux needs the platform dependencies
listed in JUCE's own setup documentation.

```sh
git clone --depth 1 https://github.com/juce-framework/JUCE.git JUCE
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
```

Artifacts: `build/KWBuilder_artefacts/Release/` for the supplied build layout.
Some single-config generators use a different configuration subfolder: inspect
`build/KWBuilder_artefacts/` if needed. CMake builds VST3 and Standalone, not VST2.
The standalone requires selecting a working audio input/output device. For live
monitoring, use headphones and avoid feeding speakers back into the input.

## Builder controls

- GAIN: continuous normalized interstage drive/level setting; not calibrated to
  the original pot's physical taper. OUTPUT LEVEL: -24 to +12 dB, default -6 dB.
- SOURCE VOICE: Calm, Sullen, Moody, Angry. Default Sullen / voice 2.
- OUTPUT PATH: Whisper / Scream. Default Scream.
- C1: 47 / 100 / 220 nF. 100 is reference.
- Q1 BYPASS CAP: Follow voice / Off / 100 nF / 1 uF / 22 uF.
  Follow voice selects none / 100 nF / 1 uF / 22 uF for voices 1 / 2 / 3 / 4.
- R4: 6.8 / 10 / 15 kOhm. 10 is reference.
- C4: 10 / 22 / 47 nF. 22 is reference.
- C3: 10 / 22 / 47 nF. 22 is the corrected reference from the perfboard BOM.
- R8: 680 Ohm / 1 kOhm / 1.5 kOhm. 1 kOhm is reference.
- INPUT LOAD: 1 / 2 MOhm. This changes an approximate input filter load only;
  it does not simulate your guitar pickup/cable or a complete loading network.
- DEVICE TEXTURE: 0.5–1.5 behavioral drive multiplier; NOT a transistor type,
  IDSS/VP selector or measured device sample.

Scream / 2, / 3 and / 4 buttons reset ALL controls to their reference defaults
and select the named voice. RESET STOCK uses Scream / 2. Here “stock” describes
reference control choices, not proof of stock-pedal sound. DAW state/preset
saving stores every parameter. No separate file-preset browser is implemented.

## Audio / DSP structure

Input coupling -> approximate source-bypass frequency weighting -> bounded
asymmetric stage 1 -> interstage coupling and GAIN -> pot-dependent C3 low-pass
-> C4 coupling -> bounded asymmetric stage 2 -> C5 coupling -> selected output
path -> OUTPUT LEVEL -> protection ceiling -> downsampling.

JUCE 4x oversampling covers the effect core. Latency is reported. Mono-to-mono
and stereo-to-stereo are supported with independent channel states. Blocks are
chunked to fixed prepared capacity. No audio-thread file IO or deliberate heap
allocation is performed in the core loop. Continuous targets are smoothed;
stepped choices still change at thresholds and are NOT guaranteed click-free.

Important simplifications:
- Stage nonlinearity is offset tanh with DC rejection; not JFET equations.
- R4/R8 are bounded drive/shape proxies, not a coupled bias recalculation.
- Source bypass uses a first-order frequency-dependent proxy. Effective loads
  of coupling networks are assumed rather than solved.
- C2 is fixed 22 nF. R1/R3/R5/R6/R7/R9/R11 are not all explicitly solved.
- The C3/pot relationship is an approximation. Pot taper/loading is not exact.
- The fixed output protection clamps at full scale before downsampling. It is
  not the pedal's clipping circuit, and extreme settings can audibly activate it.
  Filtering can create small reconstructed overshoots; keep monitoring low.
- No low/high rolloff cutoff is falsely declared to be exactly 100 Hz / 5.5 kHz.
- No MIDI, auto level matching, response display, A/B slot system, physical DC
  meters or enhanced LEAN EQ is included in this initial source package.
- The UI is a functional 12-control grid, not a pixel-perfect rendering of the
  earlier image. All circuit-part alternatives beyond stock are experiments.

## Your reference recording

`reference_notes.json` records metadata and your section order:
Scream voice 2, then 3, then 4. The MP3 is not redistributed in this package.
It has NOT been segmented by guessing equal thirds. No listening or sound-match
claim is made from metadata. No model fitting/training was performed.

For trustworthy matching, provide the matching dry DI (same performance), exact
section start/end times, pedal gain position, recording gain/chain, and whether
an amp/cab/plugin follows the pedal. Unprocessed WAV is preferable. If there is
no matching DI, make a new paired capture by reamping a saved dry take.

## Next engineering steps

1. Build / plugin validation / host testing and listen at conservative levels.
2. Verify the full original schematic and device operating points.
3. Replace proxies with a validated circuit-oriented model, or label the project
   permanently as an inspired effect rather than a hardware-building predictor.
4. Calibrate clean input volts per digital full scale and device parameters.
5. Compare against paired captures and test 2/3/4 plus Whisper and Calm.
6. Add preset UI, stage scopes and circuit-board artwork only after the core is
   reliable. Never show fake DC readouts or measured curves.

Prototype code is MIT licensed (LICENSE). Third-party JUCE and VST3-related
licenses/conditions are separate; this package does not grant their rights.
