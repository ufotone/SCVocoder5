#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>

#if defined(__ARM_NEON) || defined(__NEON__)
#include <arm_neon.h>
#define AGE_CARRIER_USE_NEON 1
#else
#define AGE_CARRIER_USE_NEON 0
#endif

#include "unit.h"

// AGE Carrier: original six-note live synth plus single-trigger chord modes.
// Oscillator and ADR envelope derived from AGE Carrier v0.1.1.
class Synth {
 public:
  enum { Attack, Decay, Release, Level, Chord, Root, Octave, Mode, Bass, ParamCount };

  static const int ChordCount = 40;

  static int minimum(int parameter) {
    const int values[] = {1, 1, 5, -24, 0, 0, 1, 0, 0};
    return parameter >= 0 && parameter < ParamCount ? values[parameter] : 0;
  }
  static int maximum(int parameter) {
    const int values[] = {200, 1000, 2000, 0, ChordCount - 1, 11, 6, 2, 12};
    return parameter >= 0 && parameter < ParamCount ? values[parameter] : 0;
  }
  static int initial(int parameter) {
    const int values[] = {5, 80, 120, 0, 0, 0, 3, 0, 0};
    return parameter >= 0 && parameter < ParamCount ? values[parameter] : 0;
  }

  Synth() {
    for (int i = 0; i < ParamCount; ++i) params_[i].store(initial(i));
    Reset();
  }

  int8_t Init(const unit_runtime_desc_t* descriptor) {
    if (!descriptor) return k_unit_err_undef;
    if (descriptor->samplerate != 48000) return k_unit_err_samplerate;
    if (descriptor->output_channels != 2) return k_unit_err_geometry;
    Reset();
    return k_unit_err_none;
  }

  void Teardown() {}
  void Suspend() {}
  void Resume() { Reset(); }

  void Reset() {
    for (int i = 0; i < kVoices; ++i) {
      phase_[i] = increment_[i] = envelope_[i] = velocity_[i] = release_step_[i] = 0.f;
      note_[i] = 0;
      stage_[i] = Off;
      age_[i] = 0;
    }
    pitch_ratio_ = 1.f;
    level_ = decibels(float(getParameterValue(Level)));
    dc_input_ = dc_output_ = 0.f;
    age_counter_ = 1;
    last_note_ = 60;
    trigger_ = -1;
    active_mode_ = getParameterValue(Mode);
  }

  void Render(float* out, size_t frames) {
    syncMode();
    const int renderVoices = active_mode_ == 0 ? kVoices : 4;
    const float attackStep = 1.f / (48.f * float(getParameterValue(Attack)));
    const float decayStep = (1.f - kSustain) / (48.f * float(getParameterValue(Decay)));
    const float targetLevel = decibels(float(getParameterValue(Level)));
    const float normalization = activeNormalization();

    for (size_t frame = 0; frame < frames; ++frame) {
      level_ += (targetLevel - level_) * .004158f;

      alignas(16) float amplitude[kVoices];
      for (int voice = 0; voice < renderVoices; ++voice) {
        switch (stage_[voice]) {
          case AttackStage:
            envelope_[voice] += attackStep;
            if (envelope_[voice] >= 1.f) {
              envelope_[voice] = 1.f;
              stage_[voice] = DecayStage;
            }
            break;
          case DecayStage:
            envelope_[voice] -= decayStep;
            if (envelope_[voice] <= kSustain) {
              envelope_[voice] = kSustain;
              stage_[voice] = SustainStage;
            }
            break;
          case ReleaseStage:
            envelope_[voice] -= release_step_[voice];
            if (envelope_[voice] <= 0.f) {
              envelope_[voice] = 0.f;
              increment_[voice] = 0.f;
              stage_[voice] = Off;
            }
            break;
          default:
            break;
        }
        amplitude[voice] = envelope_[voice] * velocity_[voice] * normalization * level_;
      }

      float sample = 0.f;
      #if AGE_CARRIER_USE_NEON
      float32x4_t sum = vdupq_n_f32(0.f);
      const float32x4_t one = vdupq_n_f32(1.f);
      const float32x4_t two = vdupq_n_f32(2.f);
      float32x4_t phase = vaddq_f32(vld1q_f32(phase_), vld1q_f32(increment_));
      phase = vsubq_f32(phase, vbslq_f32(vcgeq_f32(phase, one), one,
                                         vdupq_n_f32(0.f)));
      vst1q_f32(phase_, phase);
      const float32x4_t saw = vsubq_f32(vmulq_f32(phase, two), one);
      sum = vmlaq_f32(sum, saw, vld1q_f32(amplitude));
      float32x2_t pair = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
      pair = vpadd_f32(pair, pair);
      sample = vget_lane_f32(pair, 0);
      for (int voice = 4; voice < renderVoices; ++voice) {
        phase_[voice] += increment_[voice];
        if (phase_[voice] >= 1.f) phase_[voice] -= 1.f;
        sample += (2.f * phase_[voice] - 1.f) * amplitude[voice];
      }
      #else
      for (int voice = 0; voice < renderVoices; ++voice) {
        phase_[voice] += increment_[voice];
        if (phase_[voice] >= 1.f) phase_[voice] -= 1.f;
        sample += (2.f * phase_[voice] - 1.f) * amplitude[voice];
      }
      #endif

      // One-pole DC blocker, followed by a bounded soft limiter.
      const float dcFree = sample - dc_input_ + .995f * dc_output_;
      dc_input_ = sample;
      dc_output_ = dcFree;
      const float output = limit(dcFree);
      out[frame * 2] = output;
      out[frame * 2 + 1] = output;
    }
  }

  void setParameter(uint8_t index, int32_t value) {
    if (index < ParamCount)
      params_[index].store(std::max(minimum(index), std::min(maximum(index), int(value))),
                           std::memory_order_relaxed);
  }
  int32_t getParameterValue(uint8_t index) const {
    return index < ParamCount ? params_[index].load(std::memory_order_relaxed) : 0;
  }
  const char* getParameterStrValue(uint8_t id, int32_t value) const {
    static const char* roots[] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    static const char* sources[] = {"LIVE6", "NOTE", "FIXED"};
    if (value < minimum(id) || value > maximum(id)) return "?";
    if (id == Chord) return definition(value).name;
    if (id == Root) return roots[value];
    if (id == Bass) return value == 0 ? "OFF" : roots[value - 1];
    if (id == Mode) return sources[value];
    return "";
  }
  const uint8_t* getParameterBmpValue(uint8_t, int32_t) const { return nullptr; }

  void NoteOn(uint8_t note, uint8_t velocity) {
    syncMode();
    if (note > 127) return;
    if (velocity == 0) { NoteOff(note); return; }
    if (active_mode_ == 0) {
      startNote(note, velocity);
      last_note_ = note;
      return;
    }
    // Latest trigger owns the entire chord; an old note-off cannot stop it.
    for (int i = 0; i < kVoices; ++i) {
      stage_[i] = Off;
      envelope_[i] = increment_[i] = velocity_[i] = phase_[i] = 0.f;
    }
    int base = active_mode_ == 2
        ? 12 * (getParameterValue(Octave) + 1) + getParameterValue(Root) : note;
    const int chord = getParameterValue(Chord);
    int notes[kVoices];
    const int count = voicedNotes(base, chord, getParameterValue(Bass), notes);
    for (int i = 0; i < count; ++i) startNote(uint8_t(notes[i]), velocity);
    trigger_ = note;
    last_note_ = note;
  }

  void startNote(uint8_t note, uint8_t velocity) {
    if (note > 127) return;
    if (velocity == 0) { NoteOff(note); return; }
    int selected = -1;
    for (int i = 0; i < kVoices; ++i) {
      if (stage_[i] != Off && note_[i] == note) { selected = i; break; }
    }
    if (selected < 0) {
      for (int i = 0; i < kVoices; ++i) {
        if (stage_[i] == Off) { selected = i; break; }
      }
    }
    if (selected < 0) {
      selected = 0;
      for (int i = 1; i < kVoices; ++i)
        if (age_[i] < age_[selected]) selected = i;
    }

    note_[selected] = note;
    phase_[selected] = 0.f;
    increment_[selected] = noteIncrement(note);
    envelope_[selected] = 0.f;
    velocity_[selected] = .35f + .65f * (float(velocity) / 127.f);
    release_step_[selected] = 0.f;
    stage_[selected] = AttackStage;
    age_[selected] = age_counter_++;
  }

  void NoteOff(uint8_t note) {
    syncMode();
    if (active_mode_ == 0) {
      for (int i = 0; i < kVoices; ++i)
        if (stage_[i] != Off && note_[i] == note) beginRelease(i);
      return;
    }
    if (trigger_ != note) return;
    for (int i = 0; i < kVoices; ++i) beginRelease(i);
    trigger_ = -1;
  }

  void GateOn(uint8_t velocity) { NoteOn(last_note_, velocity); }
  void GateOff() {
    syncMode();
    if (active_mode_ == 0) NoteOff(last_note_);
    else if (trigger_ >= 0) NoteOff(uint8_t(trigger_));
  }
  void AllNoteOff() { Reset(); }

  void PitchBend(uint16_t bend) {
    const float normalized = (float(bend) - 8192.f) / 8192.f;
    pitch_ratio_ = std::pow(2.f, (2.f * normalized) / 12.f);
    for (int i = 0; i < kVoices; ++i)
      if (stage_[i] != Off) increment_[i] = noteIncrement(note_[i]);
  }
  void ChannelPressure(uint8_t) {}
  void Aftertouch(uint8_t, uint8_t) {}
  void LoadPreset(uint8_t) {}
  uint8_t getPresetIndex() const { return 0; }
  static const char* getPresetName(uint8_t) { return ""; }

 private:
  enum Stage : uint8_t { Off, AttackStage, DecayStage, SustainStage, ReleaseStage };
  static const int kVoices = 6;
 public:
  static int chordSize(int chord) {
    return chord >= 0 && chord < ChordCount ? definition(chord).count : 0;
  }
  static int chordInterval(int chord, int voice) {
    return chord >= 0 && chord < ChordCount && voice >= 0 && voice < chordSize(chord)
        ? definition(chord).intervals[voice] : 0;
  }
 private:
  struct ChordDefinition { const char* name; int count; int intervals[4]; };
  static const ChordDefinition& definition(int chord) {
    // Append new types so v0.2.0's saved chord IDs retain their meanings.
    static const ChordDefinition chords[ChordCount] = {
      {"MAJ",3,{0,4,7}}, {"MIN",3,{0,3,7}}, {"DIM",3,{0,3,6}},
      {"AUG",3,{0,4,8}}, {"SUS2",3,{0,2,7}}, {"SUS4",3,{0,5,7}},
      {"7",4,{0,4,7,10}}, {"MAJ7",4,{0,4,7,11}},
      {"MIN7",4,{0,3,7,10}}, {"M7B5",4,{0,3,6,10}},
      {"MIN6",4,{0,3,7,9}}, {"MINb6",4,{0,3,7,8}},
      {"MAJ6",4,{0,4,7,9}}, {"DIM7",4,{0,3,6,9}},
      {"MMAJ7",4,{0,3,7,11}}, {"ADD9",4,{0,4,7,14}},
      {"MADD9",4,{0,3,7,14}}, {"9",4,{0,4,10,14}},
      {"MAJ9",4,{0,4,11,14}}, {"MIN9",4,{0,3,10,14}},
      {"7SUS4",4,{0,5,7,10}}, {"6/9",4,{0,4,9,14}},
      {"M6/9",4,{0,3,9,14}},
      {"7b5",4,{0,4,6,10}},
      {"7#5",4,{0,4,8,10}},
      {"M7#5",4,{0,4,8,11}},
      {"7b9",4,{0,4,10,13}},
      {"7#9",4,{0,4,10,15}},
      {"7#11",4,{0,4,10,18}},
      {"7b13",4,{0,4,10,20}},
      {"13",4,{0,4,10,21}},
      {"M13",4,{0,4,11,21}},
      {"m13",4,{0,3,10,21}},
      {"11",4,{0,10,14,17}},
      {"m11",4,{0,3,10,17}},
      {"7SUS2",4,{0,2,7,10}},
      {"M7S2",4,{0,2,7,11}},
      {"M7S4",4,{0,5,7,11}},
      {"ADD11",4,{0,4,7,17}},
      {"MAD11",4,{0,3,7,17}}
    };
    return chords[chord];
  }
  static int voicedNotes(int base, int chord, int bass, int* notes) {
    const int count = chordSize(chord);
    const int top = chordInterval(chord, count - 1);
    while (base + top > 127) base -= 12;
    int bassNote = -1;
    if (bass > 0) {
      bassNote = (base / 12) * 12 + bass - 1;
      while (bassNote > base) bassNote -= 12;
      if (bassNote < 0) { base += 12; bassNote += 12; }
    }
    int omit = -1;
    bool bassIsChordTone = false;
    for (int i = 0; i < count; ++i)
      if (bassNote >= 0 && (base + chordInterval(chord, i)) % 12 == bassNote % 12)
        bassIsChordTone = true;
    if (bassNote >= 0 && !bassIsChordTone && count == 4) {
      // Keep the selected bass and defining extensions. Prefer omitting the
      // unaltered fifth; otherwise use a rootless upper voicing.
      omit = 0;
      for (int i = 1; i < count; ++i)
        if (chordInterval(chord, i) == 7) omit = i;
    }
    int n = 0;
    if (bassNote >= 0) notes[n++] = bassNote;
    for (int i = 0; i < count; ++i) {
      const int note = base + chordInterval(chord, i);
      // Generated voicings, including a slash bass, never exceed four notes.
      if (i != omit && (bassNote < 0 || note % 12 != bassNote % 12)) notes[n++] = note;
    }
    return n;
  }
  static constexpr float kSustain = .85f;

  void syncMode() {
    if (active_mode_ != getParameterValue(Mode)) Reset();
  }
  static float decibels(float value) { return std::pow(10.f, value / 20.f); }
  static float limit(float value) {
    const float magnitude = std::fabs(value);
    return magnitude <= .8f
               ? value
               : std::copysign(.8f + .2f * (magnitude - .8f) / (magnitude - .6f), value);
  }
  float noteIncrement(uint8_t note) const {
    const float frequency = 440.f * std::pow(2.f, (float(note) - 69.f) / 12.f);
    return frequency * pitch_ratio_ / 48000.f;
  }
  void beginRelease(int voice) {
    if (stage_[voice] == Off || stage_[voice] == ReleaseStage) return;
    const float samples = 48.f * float(getParameterValue(Release));
    release_step_[voice] = envelope_[voice] / std::max(1.f, samples);
    stage_[voice] = ReleaseStage;
  }
  float activeNormalization() const {
    int active = 0;
    for (int i = 0; i < kVoices; ++i) if (stage_[i] != Off) ++active;
    return active > 0 ? .9f / std::sqrt(float(active)) : 0.f;
  }

  std::atomic<int> params_[ParamCount];
  alignas(16) float phase_[kVoices]{}, increment_[kVoices]{};
  alignas(16) float envelope_[kVoices]{}, velocity_[kVoices]{}, release_step_[kVoices]{};
  uint8_t note_[kVoices]{}, stage_[kVoices]{};
  uint32_t age_[kVoices]{};
  uint32_t age_counter_ = 1;
  float pitch_ratio_ = 1.f;
  float level_ = 1.f;
  float dc_input_ = 0.f, dc_output_ = 0.f;
  uint8_t last_note_ = 60;
  int trigger_ = -1;
  int active_mode_ = 0;
};
