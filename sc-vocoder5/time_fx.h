#pragma once
#include <algorithm>
#include <cmath>

// Fixed-storage time effects at 48 kHz. No allocation or locks in rendering.
class TimeEffects {
 public:
  void reset() {
    std::fill(flange_[0], flange_[0] + kFlangeSize, 0.f);
    std::fill(flange_[1], flange_[1] + kFlangeSize, 0.f);
    clearReverb();
    flange_write_ = 0;
    flange_valid_ = 0;
    flange_phase_ = 0.f;
    flange_mix_ = reverb_mix_ = 0.f;
    reverb_active_ = false;
  }

  void prepare(int flange, int rate, int reverb, int decayMs) {
    target_flange_ = flange * .01f;
    target_reverb_ = reverb * .01f;
    flange_step_ = (.05f + 1.45f * rate * .01f) / 48000.f;
    if (decayMs != decay_ms_) {
      decay_ms_ = decayMs;
      for (int i = 0; i < 4; ++i)
        reverb_gain_[i] = std::pow(.001f, float(length(i)) / (48.f * decayMs));
    }
    if (target_reverb_ > 0.f && !reverb_active_) {
      clearReverb();
      reverb_active_ = true;
    }
  }

  void tick(float& left, float& right) {
    smooth(flange_mix_, target_flange_);
    smooth(reverb_mix_, target_reverb_);
    if (flange_mix_ > 1e-5f) processFlanger(left, right);
    else flange_valid_ = 0;
    if (reverb_mix_ > 1e-5f) processReverb(left, right);
    else if (target_reverb_ == 0.f) reverb_active_ = false;
  }

 private:
  static const int kFlangeSize = 512;
  static int length(int i) {
    static const int lengths[] = {1493, 1789, 2137, 2411};
    return lengths[i];
  }
  static void smooth(float& value, float target) {
    value += (target - value) * .004158f;
  }
  static float bounded(float value) { return std::max(-4.f, std::min(4.f, value)); }
  static float read(const float* buffer, int size, int write, int valid, float delay) {
    // A validity counter avoids replaying stale samples after an effect was off.
    if (valid < int(delay) + (delay > float(int(delay)) ? 1 : 0)) return 0.f;
    float position = float(write) - delay;
    if (position < 0.f) position += size;
    const int first = int(position);
    const int second = (first + 1) & (size - 1);
    return buffer[first] + (buffer[second] - buffer[first]) * (position - first);
  }
  void processFlanger(float& left, float& right) {
    // 0.5--4.5 ms, opposed triangle modulation; bounded fixed feedback.
    const float triangle = 4.f * std::fabs(flange_phase_ - .5f) - 1.f;
    const float l = read(flange_[0], kFlangeSize, flange_write_, flange_valid_, 120.f + 96.f * triangle);
    const float r = read(flange_[1], kFlangeSize, flange_write_, flange_valid_, 120.f - 96.f * triangle);
    flange_[0][flange_write_] = bounded(left + .45f * l);
    flange_[1][flange_write_] = bounded(right + .45f * r);
    left += (l - left) * (.5f * flange_mix_);
    right += (r - right) * (.5f * flange_mix_);
    // Compensate the linear dry/wet blend: up to sqrt(2), +3.01 dB.
    // Following the smoothed amount keeps ON/OFF and knob changes continuous.
    // Apply after the feedback write, so compensation cannot raise loop gain.
    const float makeup = 1.f + .41421356237f * flange_mix_;
    left *= makeup;
    right *= makeup;
    flange_write_ = (flange_write_ + 1) & (kFlangeSize - 1);
    flange_valid_ = std::min(kFlangeSize, flange_valid_ + 1);
    flange_phase_ += flange_step_;
    if (flange_phase_ >= 1.f) flange_phase_ -= 1.f;
  }
  void clearReverb() {
    for (int i = 0; i < 4; ++i) {
      std::fill(reverb_[i], reverb_[i] + 4096, 0.f);
      reverb_write_[i] = 0;
      damp_[i] = 0.f;
      std::fill(diffuser_[i], diffuser_[i] + 512, 0.f);
      diffuser_write_[i] = 0;
    }
  }
  float diffuse(int i, float input) {
    const int n = (i & 1) ? 337 : 113;
    const float delayed = diffuser_[i][diffuser_write_[i]];
    const float output = delayed - .5f * input;
    diffuser_[i][diffuser_write_[i]] = input + .5f * output;
    if (++diffuser_write_[i] == n) diffuser_write_[i] = 0;
    return output;
  }
  void processReverb(float& left, float& right) {
    const float inputL = diffuse(1, diffuse(0, .35f * bounded(left)));
    const float inputR = diffuse(3, diffuse(2, .35f * bounded(right)));
    float y[4];
    for (int i = 0; i < 4; ++i) {
      y[i] = reverb_[i][reverb_write_[i]];
      damp_[i] += .25f * (y[i] - damp_[i]);
      if (std::fabs(damp_[i]) < 1e-20f) damp_[i] = 0.f;
    }
    // Orthogonal Hadamard feedback network; all loop gains stay below one.
    const float feedback[4] = {
      .5f * (damp_[0] + damp_[1] + damp_[2] + damp_[3]),
      .5f * (damp_[0] - damp_[1] + damp_[2] - damp_[3]),
      .5f * (damp_[0] + damp_[1] - damp_[2] - damp_[3]),
      .5f * (damp_[0] - damp_[1] - damp_[2] + damp_[3])
    };
    const float injection[4] = {inputL, inputR, -inputR, inputL};
    for (int i = 0; i < 4; ++i) {
      reverb_[i][reverb_write_[i]] = bounded(injection[i] + feedback[i] * reverb_gain_[i]);
      if (++reverb_write_[i] == length(i)) reverb_write_[i] = 0;
    }
    left += reverb_mix_ * .5f * (y[0] + y[2]);
    right += reverb_mix_ * .5f * (y[1] - y[3]);
  }

  float flange_[2][kFlangeSize]{};
  float reverb_[4][4096]{}, diffuser_[4][512]{};
  float damp_[4]{}, reverb_gain_[4]{};
  int reverb_write_[4]{}, diffuser_write_[4]{};
  int flange_write_ = 0, flange_valid_ = 0;
  int decay_ms_ = -1;
  float flange_phase_ = 0.f, flange_step_ = .35f / 48000.f;
  float flange_mix_ = 0.f, reverb_mix_ = 0.f;
  float target_flange_ = 0.f, target_reverb_ = 0.f;
  bool reverb_active_ = false;
};
