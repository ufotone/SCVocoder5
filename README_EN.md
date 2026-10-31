# SCVocoder5

## 🌐 Language / 言語

### [日本語](README.md)　|　[English](README_EN.md)

Current language: **English**

---

A vocoder and dedicated carrier synth for the KORG drumlogue.

[Download the latest release](https://github.com/ufotone/SCVocoder5/releases/latest)

## Included Units

- `SC Vocoder 5 v0.7.0`: Vocoder for the Master FX slot
- `AGE Carrier v0.4.0`: Saw-wave chord synth for the MULTI track

## Installation

1. Download and extract the distribution ZIP from Releases.
2. Start the drumlogue in USB mass storage mode.
3. Copy the following files to the drumlogue.

|File|Destination|
|---|---|
|`sc_vocoder5.drmlgunit`|`Units/MasterFXs/`|
|`age_carrier.drmlgunit`|`Units/Synths/`|

4. Restart the drumlogue normally.

## Basic Setup

1. Select `AGE Carrier` as the user synth on the MULTI track.
2. Connect a MIDI keyboard and set its MIDI channel as described below.
3. Connect a microphone with a preamp to AUDIO IN.
4. Hold `SHIFT` and press the `BD` button, then set AUDIO IN `ROUTE` to `SIDECH`.
5. Select `SC Vocoder 5` as the Master FX.
6. Hold a chord on the keyboard while speaking into the microphone.
7. Start with the following settings.

|Parameter|Recommended value|
|---|---:|
|RANGE|AGE|
|MOD GAIN|+35dB|
|UNVOICED|20%|
|CHORUS|35%|
|MIX|100%|
|VOICE DRY|0%|

Set `MONITOR` to `CARRIER` to hear the synth, `MODULATOR` to hear the microphone, or `VOCODER` to hear the vocoder output.

## Microphone and AUDIO IN Setup

As a general rule, use a self-powered microphone with a built-in preamp, or connect the microphone through an external mic preamp. A typical passive microphone connected directly to the drumlogue may not provide enough level. If the microphone requires power, use its own battery, power supply, or an external preamp.

1. Turn down the microphone or preamp output, then connect it to the drumlogue AUDIO IN.
2. Hold `SHIFT` and press the `BD` button to open the AUDIO IN settings.
3. Set `ROUTE` to `SIDECH`.
4. Start with a low `LEVEL` setting and raise it gradually.
5. Set SC Vocoder 5 `MONITOR` to `MODULATOR` and check the microphone signal.
6. Speak at the loudest level you expect to use, then adjust the microphone output and `LEVEL` so that the signal does not distort.
7. Return `MONITOR` to `VOCODER` and adjust `MOD GAIN` if necessary.

Do not immediately turn every gain control to maximum when the signal is too quiet. Raise the microphone output, AUDIO IN `LEVEL`, and `MOD GAIN` in that order. If the sound clips, becomes harsh, or jumps in level on consonants, lower the microphone output or AUDIO IN `LEVEL`. Excessive `MOD GAIN` also raises noise and distortion.

## MIDI Keyboard Setup

The following example sets the drumlogue MIDI channel range to `1-12`. With this setting, the MULTI track is played on MIDI channel 11.

1. Connect a MIDI keyboard by USB or MIDI IN.
2. Hold `SHIFT` and press the `LIVE` button to enter GLOBAL mode.
3. Open `MIDI ROUTING` with button 7.
4. Set `CH` to `1-12`.
5. Set the keyboard transmit channel to `11`.
6. Select `AGE Carrier` on the MULTI track and confirm that it plays from the keyboard.

Refer to the keyboard manual for its MIDI channel setting procedure. If playing the keyboard produces no sound, confirm that the keyboard transmit channel matches the drumlogue MULTI track channel.

## SC Vocoder 5

### Page 1

|Parameter|Range|Default|Description|
|---|---:|---:|---|
|FORMANT|−12 to +12|0|Shifts the voice formant position up or down.|
|RELEASE|1 to 500ms|100ms|Controls how long the sound remains after the voice stops.|
|RANGE|FULL / AGE|AGE|Selects the frequency range used for analysis. AGE is recommended for normal use.|
|MIX|0 to 100%|100%|Adjusts the balance between the dry and vocoded signals.|

### Page 2

|Parameter|Range|Default|Description|
|---|---:|---:|---|
|ATTACK|1 to 50ms|4ms|Controls how quickly the vocoder responds to the voice.|
|MOD GAIN|−24 to +50dB|+35dB|Adjusts the microphone input level inside the vocoder.|
|OUTPUT|−24 to +12dB|0dB|Adjusts the final output level.|
|UNVOICED|0 to 100%|0%|Adds unvoiced consonant components such as “s” and “sh.” Start around 20%.|

### Page 3

|Parameter|Options / Range|Default|Description|
|---|---:|---:|---|
|INPUT|SIDECHAIN / L/R SPLIT|SIDECHAIN|Selects the input method. In L/R SPLIT mode, the left channel is the carrier and the right channel is the modulator.|
|MONITOR|VOCODER / CARRIER / MODULATOR|VOCODER|Selects which signal to monitor.|
|VOICE DRY|0 to 100%|0%|Adds the unprocessed microphone signal.|
|WIDTH|0 to 100%|100%|Adjusts the stereo width of the vocoder output.|

### Page 4

|Parameter|Range|Default|Description|
|---|---:|---:|---|
|CHORUS|0 to 100%|35%|Adds width to the vocoder sound.|
|FLANGER|0 to 100%|0%|Flanger amount.|
|FLG RATE|0 to 100%|20%|Flanger speed, mapped to 0.05–1.5Hz.|
|REVERB|0 to 100%|0%|Stereo reverb amount.|

### Page 5

|Parameter|Range|Default|Description|
|---|---:|---:|---|
|REV TIME|300 to 5000ms|1800ms|Approximate reverb decay time.|

Version 0.7.0 removes the dedicated echo controls, processing, state and buffers. There are 17 controls and no compatibility placeholders.

Processing order: CHORUS → FLANGER → REVERB. FLANGER automatically applies makeup gain according to its amount, up to approximately +3.01dB. Compensation is applied outside the feedback loop; the final safety limiter remains active. OUTPUT retains its +12dB maximum. Added effects default to OFF. The MIX dry path, VOICE DRY and CARRIER / MODULATOR monitoring bypass the added effects. Host tests and the ARM build pass; hardware loudness, sound quality and CPU load have not been verified for this version.

## AGE Carrier

Version 0.4.0 extends the original AGE Carrier. Six-note live playing remains the default.

|MODE|Behavior|
|---|---|
|LIVE6 (default)|Plays incoming notes, with up to six simultaneous voices.|
|NOTE|Uses one input note as the root of the selected chord.|
|FIXED|Uses ROOT, OCTAVE and CHORD selected on the unit; a note triggers the complete chord.|

NOTE and FIXED generate one chord per trigger and disable six-note live playing. New triggers replace the previous chord. Generated voicings use at most four voices, including the bass.

- Page 1: ATTACK / DECAY / RELEASE / LEVEL, with original ranges, defaults and IDs.
- Page 2: CHORD / ROOT / OCTAVE / MODE.
- Page 3: BASS, OFF (default) or an absolute note name.

CHORD offers 40 types: MAJ / MIN / DIM / AUG / SUS2 / SUS4 / 7 / MAJ7 / MIN7 / M7B5 / MIN6 / MINb6 / MAJ6 / DIM7 / MMAJ7 / ADD9 / MADD9 / 9 / MAJ9 / MIN9 / 7SUS4 / 6/9 / M6/9 / 7b5 / 7#5 / M7#5 / 7b9 / 7#9 / 7#11 / 7b13 / 13 / M13 / m13 / 11 / m11 / 7SUS2 / M7S2 / M7S4 / ADD11 / MAD11. MAJ is the default; existing chord IDs are retained. Ninth and 6/9 types now omit the fifth; new 11th and 13th types also use explicit four-note voicings with omissions. Chord modes render four oscillator/envelope slots; LIVE6 retains the original six. Hardware CPU savings have not been measured.

BASS places the selected pitch at or below the chord root. A chord-tone bass moves that tone into the bass; a non-chord bass adds one voice. If that would require five notes, the natural fifth is omitted, or the root if no natural fifth is present, to retain a four-note limit. BASS is an absolute pitch class even in NOTE mode. Black keys are displayed as Db / Eb / Gb / Ab / Bb. Changes take effect at the next trigger.

For this progression, set MODE=FIXED, OCTAVE=3, BASS=G:

|Chord|ROOT|CHORD|Generated notes|
|---|---|---|---|
|Gm7|G|MIN7|G3–Bb3–D4–F4|
|Gm6|G|MIN6|G3–Bb3–D4–E4|
|Gm(b6)|G|MINb6|G3–Bb3–D4–Eb4|
|AbM7/G|Ab|MAJ7|G3–Ab3–C4–Eb4|

Select each chord using ROOT, CHORD and BASS; switch ROOT and CHORD as shown to play the progression. The display name remains AGE Carrier and the file remains `age_carrier.drmlgunit`. The original unit ID and first eight parameter IDs are retained.

## Troubleshooting

- If the microphone cannot be heard, check AUDIO IN `ROUTE=SIDECH`, microphone power, battery, and preamp, then test with `MONITOR=MODULATOR`.
- If AGE Carrier cannot be heard, select `MONITOR=CARRIER` and check the MULTI track selection and MIDI channel.
- If only the microphone is heard, check that `MONITOR` is not set to `MODULATOR` and that `VOICE DRY` is not raised.
- If only the synth is heard, check that `MONITOR` is not set to `CARRIER` and that AUDIO IN is routed to `SIDECH`.
- If the vocoder output is too quiet, check the microphone preamp, AUDIO IN `LEVEL`, and `MOD GAIN`, in that order.
- If the sound is distorted, lower the microphone output and AUDIO IN `LEVEL`, then adjust `MOD GAIN` and `OUTPUT`.
- If abnormal behavior continues, remove the unit and restart the drumlogue.

## License

MIT License. Portions derived from KORG templates are covered by the KORG BSD 3-Clause License.
