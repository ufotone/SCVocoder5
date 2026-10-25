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

// Six-note, single-saw-per-note carrier synth. The fixed 85% sustain keeps
// the parameter set to ADR + level. No filter, modulation, FX, or allocation.
class Synth {
 public:
  enum { Attack, Decay, Release, Level, ParamCount };

  static int minimum(int parameter) {
    const int values[] = {1, 1, 5, -24};
    return parameter >= 0 && parameter < ParamCount ? values[parameter] : 0;
  }
  static int maximum(int parameter) {
    const int values[] = {200, 1000, 2000, 0};
    return parameter >= 0 && parameter < ParamCount ? values[parameter] : 0;
  }
  static int initial(int parameter) {
    const int values[] = {5, 80, 120, 0};
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
  }

  void Render(float* out, size_t frames) {
    const float attackStep = 1.f / (48.f * float(getParameterValue(Attack)));
    const float decayStep = (1.f - kSustain) / (48.f * float(getParameterValue(Decay)));
    const float targetLevel = decibels(float(getParameterValue(Level)));
    const float normalization = activeNormalization();

    for (size_t frame = 0; frame < frames; ++frame) {
      level_ += (targetLevel - level_) * .004158f;

      alignas(16) float amplitude[kVoices];
      for (int voice = 0; voice < kVoices; ++voice) {
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
      for (int voice = 4; voice < kVoices; ++voice) {
        phase_[voice] += increment_[voice];
        if (phase_[voice] >= 1.f) phase_[voice] -= 1.f;
        sample += (2.f * phase_[voice] - 1.f) * amplitude[voice];
      }
      #else
      for (int voice = 0; voice < kVoices; ++voice) {
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
  const char* getParameterStrValue(uint8_t, int32_t) const { return ""; }
  const uint8_t* getParameterBmpValue(uint8_t, int32_t) const { return nullptr; }

  void NoteOn(uint8_t note, uint8_t velocity) {
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
    last_note_ = note;
  }

  void NoteOff(uint8_t note) {
    for (int i = 0; i < kVoices; ++i) {
      if (stage_[i] != Off && note_[i] == note) beginRelease(i);
    }
  }

  void GateOn(uint8_t velocity) { NoteOn(last_note_, velocity); }
  void GateOff() { NoteOff(last_note_); }
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
  static constexpr float kSustain = .85f;

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
};
