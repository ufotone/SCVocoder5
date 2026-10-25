# SCVocoder5

[日本語ドキュメント](README.md)

A vocoder and dedicated carrier synth for the KORG drumlogue.

## Included Units

- `SC Vocoder 5 v0.5.1`: Vocoder for the Master FX slot
- `AGE Carrier v0.1.1`: Saw-wave chord synth for the MULTI track

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
|CHORUS|0 to 100%|35%|Adds width and depth to the vocoder sound.|

## AGE Carrier

A six-voice polyphonic saw-wave chord synth. It produces a sustained carrier signal for the vocoder while keys are held.

|Parameter|Range|Default|Description|
|---|---:|---:|---|
|ATTACK|1 to 200ms|5ms|Adjusts the sound onset.|
|DECAY|1 to 1000ms|80ms|Adjusts the time required to reach the sustain level.|
|RELEASE|5 to 2000ms|120ms|Adjusts how long the sound remains after the keys are released.|
|LEVEL|−24 to 0dB|0dB|Adjusts the carrier output level.|

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
