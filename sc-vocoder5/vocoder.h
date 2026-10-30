#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

#if defined(__ARM_NEON) || defined(__NEON__)
#include <arm_neon.h>
#define SC_VOCODER_USE_NEON 1
#else
#define SC_VOCODER_USE_NEON 0
#endif

#include "time_fx.h"

// Stability-first portable DSP. SDK calls are isolated in unit.cc.
// There is no allocation, locking, or filter setup inside the sample loop.
class Vocoder {
 public:
  enum { Formant, Release, Profile, Mix, Attack, ModGain, OutputGain,
         Unvoiced, InputMode, Monitor, VoiceDry, Width, Chorus, Flanger, FlangerRate, Reverb, ReverbTime, ParamCount };

  static int minimum(int p) {
    return p == ReverbTime ? 300 : (p == Formant ? -12 :
           (p == ModGain || p == OutputGain ? -24 :
           (p == Release || p == Attack ? 1 : 0)));
  }
  static int maximum(int p) {
    const int values[] = {12, 500, 1, 100, 50, 50, 12, 100, 1, 2, 100, 100, 100, 100, 100, 100, 5000};
    return p >= 0 && p < ParamCount ? values[p] : 0;
  }
  static int initial(int p) {
    // AGE is the default 12-band profile derived from ageage90's WebAudio vocoder.
    const int values[] = {0, 100, 1, 100, 4, 35, 0, 0, 0, 0, 0, 100, 35, 0, 20, 0, 1800};
    return p >= 0 && p < ParamCount ? values[p] : 0;
  }

  Vocoder() {
    for (int p = 0; p < ParamCount; ++p)
      params_[p].store(initial(p));
    reset();
  }

  void set(int p, int v) {
    if (p >= 0 && p < ParamCount)
      params_[p].store(std::max(minimum(p), std::min(maximum(p), v)),
                       std::memory_order_relaxed);
  }
  int get(int p) const {
    return p >= 0 && p < ParamCount
               ? params_[p].load(std::memory_order_relaxed)
               : 0;
  }

  void reset() {
    profile_ = get(Profile);
    configure();
    fade_ = 1.f;
    mix_ = get(Mix) * .01f;
    modgain_ = db(float(get(ModGain)));
    outgain_ = db(float(get(OutputGain)));
    voice_ = get(VoiceDry) * .01f;
    width_ = get(Width) * .01f;
    chorus_ = get(Chorus) * .01f;
    unvoiced_ = get(Unvoiced) * .01f;
    attack_ = time(float(get(Attack)));
    release_ = time(float(get(Release)));
    carrier_env_ = hp_prev_ = hp_state_ = 0.f;
    std::fill(chorus_left_, chorus_left_ + kChorusBufferSize, 0.f);
    std::fill(chorus_right_, chorus_right_ + kChorusBufferSize, 0.f);
    chorus_write_ = 0;
    chorus_phase_ = 0.f;
    chorus_warmup_ = kMaxChorusDelay;
    effects_.prepare(get(Flanger), get(FlangerRate), get(Reverb), get(ReverbTime));
    effects_.reset();
    monitor_active_ = false;
    fault_ = false;
  }

  void process(const float* in, float* out, uint32_t frames, int channels) {
    int p[ParamCount];
    for (int i = 0; i < ParamCount; ++i) p[i] = get(i);

    // Monitoring is a genuine low-cost bypass. The previous versions ran the
    // complete filter bank before replacing its output, even in monitor modes.
    if (p[Monitor] != 0) {
      monitor_active_ = true;
      processMonitor(in, out, frames, channels, p);
      return;
    }

    // Do not read stale delay contents when returning from a monitor mode.
    if (monitor_active_) {
      chorus_warmup_ = kMaxChorusDelay;
      effects_.reset();
      monitor_active_ = false;
    }

    if (fault_) reset();

    if (p[Profile] != profile_) {
      profile_ = p[Profile];
      configure();  // block boundary only, never from inside the sample loop
      fade_ = 0.f;
    }
    if (p[Formant] != mapped_formant_) updateMapping(p[Formant]);

    const float targetAttack = time(float(p[Attack]));
    const float targetRelease = time(float(p[Release]));
    const float targetMod = db(float(p[ModGain]));
    const float targetOut = db(float(p[OutputGain]));
    const bool split = p[InputMode] == 1;
    effects_.prepare(p[Flanger], p[FlangerRate], p[Reverb], p[ReverbTime]);

    for (uint32_t f = 0; f < frames; ++f) {
      fade_ = std::min(1.f, fade_ + 1.f / 480.f);
      smooth(mix_, p[Mix] * .01f);
      smooth(modgain_, targetMod);
      smooth(outgain_, targetOut);
      smooth(voice_, p[VoiceDry] * .01f);
      smooth(width_, p[Width] * .01f);
      smooth(chorus_, p[Chorus] * .01f);
      smooth(unvoiced_, p[Unvoiced] * .01f);
      smooth(attack_, targetAttack);
      smooth(release_, targetRelease);

      const float* x = in + f * channels;
      const float l = clean(x[0]);
      const float r = split ? l : clean(x[1]);
      const float rawMod = split ? clean(x[1])
                                 : (channels == 4 ? .5f * (clean(x[2]) + clean(x[3])) : 0.f);
      const float mod = rawMod * modgain_;
      const float speech = hpf_enabled_ ? mod_hpf_.tick(mod) : mod;

      #if SC_VOCODER_USE_NEON
      const float32x4_t attack4 = vdupq_n_f32(attack_);
      const float32x4_t release4 = vdupq_n_f32(release_);
      for (int i = 0; i < count_; i += 4) {
        const float32x4_t a = vabsq_f32(analysis_.tick4(i, speech));
        const float32x4_t previous = vld1q_f32(env_ + i);
        const uint32x4_t rising = vcgtq_f32(a, previous);
        const float32x4_t coefficient = vbslq_f32(rising, attack4, release4);
        const float32x4_t next = vmlaq_f32(a, coefficient, vsubq_f32(previous, a));
        vst1q_f32(env_ + i, next);
      }
      #else
      for (int i = 0; i < count_; ++i) {
        const float a = std::fabs(analysis_.tick(i, speech));
        env_[i] = (a > env_[i] ? attack_ : release_) * (env_[i] - a) + a;
      }
      #endif

      // A mono synthesis bank cuts the hot path from three filter banks to two.
      // Alternating fixed pans retain useful stereo width without another bank.
      const float carrier = .5f * (l + r) * carrier_gain_;
      for (int i = 0; i < count_; ++i) {
        const int j = map_index_[i];
        const float t = map_fraction_[i];
        const float e0 = j >= 0 && j < count_ ? env_[j] : 0.f;
        const float e1 = j + 1 >= 0 && j + 1 < count_ ? env_[j + 1] : 0.f;
        band_gain_[i] = std::min(env_cap_, env_scale_ * (e0 + (e1 - e0) * t));
      }
      float wetL = 0.f;
      float wetR = 0.f;
      #if SC_VOCODER_USE_NEON
      const float spread = .28f * width_;
      float32x4_t leftPan = vdupq_n_f32(1.f + spread);
      leftPan = vsetq_lane_f32(1.f - spread, leftPan, 1);
      leftPan = vsetq_lane_f32(1.f - spread, leftPan, 3);
      float32x4_t rightPan = vdupq_n_f32(1.f - spread);
      rightPan = vsetq_lane_f32(1.f + spread, rightPan, 1);
      rightPan = vsetq_lane_f32(1.f + spread, rightPan, 3);
      float32x4_t sumL = vdupq_n_f32(0.f);
      float32x4_t sumR = vdupq_n_f32(0.f);
      for (int i = 0; i < count_; i += 4) {
        const float32x4_t band = vmulq_f32(synthesis_.tick4(i, carrier),
                                           vld1q_f32(band_gain_ + i));
        sumL = vmlaq_f32(sumL, band, leftPan);
        sumR = vmlaq_f32(sumR, band, rightPan);
      }
      float32x2_t pairL = vadd_f32(vget_low_f32(sumL), vget_high_f32(sumL));
      float32x2_t pairR = vadd_f32(vget_low_f32(sumR), vget_high_f32(sumR));
      pairL = vpadd_f32(pairL, pairL);
      pairR = vpadd_f32(pairR, pairR);
      wetL = vget_lane_f32(pairL, 0);
      wetR = vget_lane_f32(pairR, 0);
      #else
      for (int i = 0; i < count_; ++i) {
        const float band = synthesis_.tick(i, carrier) * band_gain_[i];
        const float pan = (i & 1 ? .28f : -.28f) * width_;
        wetL += band * (1.f - pan);
        wetR += band * (1.f + pan);
      }
      #endif

      const float carrierLevel = .5f * (std::fabs(l) + std::fabs(r));
      carrier_env_ = .999f * carrier_env_ + .001f * carrierLevel;
      const float hp = mod - hp_prev_ + .75f * hp_state_;
      hp_prev_ = mod;
      hp_state_ = hp;
      const float consonant = hp * unvoiced_ * std::min(1.f, carrier_env_ * 40.f);
      wetL = wetL * fade_ * profile_gain_;
      wetR = wetR * fade_ * profile_gain_;
      processChorus(wetL, wetR);
      // Keep unvoiced consonants outside the chorus so articulation stays crisp.
      wetL += consonant;
      wetR += consonant;
      effects_.tick(wetL, wetR);

      const float yL = (1.f - mix_) * l + mix_ * wetL + voice_ * mod;
      const float yR = (1.f - mix_) * r + mix_ * wetR + voice_ * mod;
      writeSafe(out + f * 2, yL * outgain_, yR * outgain_);
    }
  }

 private:
  struct BPFBank {
    alignas(16) float b0[16]{}, b2[16]{}, a1[16]{}, a2[16]{};
    alignas(16) float z1[16]{}, z2[16]{};
    void setup(int index, float hz, float q) {
      const float w = 6.28318530718f * hz / 48000.f;
      const float alpha = std::sin(w) / (2.f * q);
      const float inv = 1.f / (1.f + alpha);
      b0[index] = alpha * inv;
      b2[index] = -b0[index];
      a1[index] = -2.f * std::cos(w) * inv;
      a2[index] = (1.f - alpha) * inv;
      z1[index] = z2[index] = 0.f;
    }
    float tick(int index, float x) {
      const float y = b0[index] * x + z1[index];
      z1[index] = -a1[index] * y + z2[index];
      z2[index] = b2[index] * x - a2[index] * y;
      if (std::fabs(z1[index]) < 1e-20f) z1[index] = 0.f;
      if (std::fabs(z2[index]) < 1e-20f) z2[index] = 0.f;
      return y;
    }
    #if SC_VOCODER_USE_NEON
    float32x4_t tick4(int index, float x) {
      const float32x4_t input = vdupq_n_f32(x);
      const float32x4_t output = vmlaq_f32(vld1q_f32(z1 + index),
                                           vld1q_f32(b0 + index), input);
      const float32x4_t nextZ1 = vmlsq_f32(vld1q_f32(z2 + index),
                                           vld1q_f32(a1 + index), output);
      const float32x4_t nextZ2 = vmlsq_f32(vmulq_f32(vld1q_f32(b2 + index), input),
                                           vld1q_f32(a2 + index), output);
      vst1q_f32(z1 + index, nextZ1);
      vst1q_f32(z2 + index, nextZ2);
      return output;
    }
    #endif
  };

  struct HighPass {
    float b0 = 1.f, b1 = 0.f, b2 = 0.f, a1 = 0.f, a2 = 0.f;
    float z1 = 0.f, z2 = 0.f;
    void setup(float hz, float q) {
      const float w = 6.28318530718f * hz / 48000.f;
      const float cosine = std::cos(w);
      const float alpha = std::sin(w) / (2.f * q);
      const float inverse = 1.f / (1.f + alpha);
      b0 = .5f * (1.f + cosine) * inverse;
      b1 = -(1.f + cosine) * inverse;
      b2 = b0;
      a1 = -2.f * cosine * inverse;
      a2 = (1.f - alpha) * inverse;
      z1 = z2 = 0.f;
    }
    float tick(float input) {
      const float output = b0 * input + z1;
      z1 = b1 * input - a1 * output + z2;
      z2 = b2 * input - a2 * output;
      return output;
    }
  };

  void processMonitor(const float* in, float* out, uint32_t frames,
                      int channels, const int* p) {
    const bool split = p[InputMode] == 1;
    const bool modulator = p[Monitor] == 2;
    const float gain = db(float(p[OutputGain])) *
                       (modulator ? db(float(p[ModGain])) : 1.f);
    for (uint32_t f = 0; f < frames; ++f) {
      const float* x = in + f * channels;
      const float l = clean(x[0]);
      const float r = split ? l : clean(x[1]);
      if (modulator) {
        const float mod = split ? clean(x[1])
                                : (channels == 4 ? .5f * (clean(x[2]) + clean(x[3])) : 0.f);
        writeSafe(out + f * 2, mod * gain, mod * gain);
      } else {
        writeSafe(out + f * 2, l * gain, r * gain);
      }
    }
  }

  static float db(float value) { return std::pow(10.f, value / 20.f); }
  static float time(float ms) { return std::exp(-1.f / (.001f * ms * 48000.f)); }
  static void smooth(float& current, float target) {
    current += (target - current) * .004158f;
  }
  static bool finite(float value) {
    union Bits { float f; uint32_t u; } bits = {value};
    return (bits.u & 0x7f800000U) != 0x7f800000U;
  }
  static float clean(float value) { return finite(value) ? value : 0.f; }
  static float limit(float value) {
    const float magnitude = std::fabs(value);
    return magnitude <= .8f
               ? value
               : std::copysign(.8f + .2f * (magnitude - .8f) / (magnitude - .6f), value);
  }

  float readChorus(const float* buffer, float delay) const {
    float position = float(chorus_write_) - delay;
    if (position < 0.f) position += float(kChorusBufferSize);
    const int first = int(position);
    const int second = (first + 1) & (kChorusBufferSize - 1);
    const float fraction = position - float(first);
    return buffer[first] + (buffer[second] - buffer[first]) * fraction;
  }

  void processChorus(float& left, float& right) {
    chorus_left_[chorus_write_] = left;
    chorus_right_[chorus_write_] = right;

    // 0.35 Hz triangle LFO, 8 ms base delay, +/-1.5 ms depth at 48 kHz.
    // Opposite delay motion creates stereo width without sine calculations.
    const float triangle = 4.f * std::fabs(chorus_phase_ - .5f) - 1.f;
    if (chorus_warmup_ > 0) {
      --chorus_warmup_;
    } else if (chorus_ > .0001f) {
      const float delayedLeft = readChorus(chorus_left_, 384.f + 72.f * triangle);
      const float delayedRight = readChorus(chorus_right_, 384.f - 72.f * triangle);
      const float wet = .35f * chorus_;
      left += (delayedLeft - left) * wet;
      right += (delayedRight - right) * wet;
    }

    chorus_write_ = (chorus_write_ + 1) & (kChorusBufferSize - 1);
    chorus_phase_ += .35f / 48000.f;
    if (chorus_phase_ >= 1.f) chorus_phase_ -= 1.f;
  }
  void writeSafe(float* destination, float left, float right) {
    if (!finite(left) || !finite(right)) {
      fault_ = true;
      destination[0] = destination[1] = 0.f;
      return;
    }
    destination[0] = limit(left);
    destination[1] = limit(right);
  }

  void configure() {
    float low = 120.f;
    float high = 8000.f;
    float q = 0.f;
    hpf_enabled_ = false;
    env_scale_ = 6.f;
    env_cap_ = 2.f;
    carrier_gain_ = 1.f;
    profile_gain_ = 1.f;
    if (profile_ == 1) {
      low = 200.f; high = 7200.f; q = 15.f;
      hpf_enabled_ = true; env_scale_ = 25.f; env_cap_ = 5.f; carrier_gain_ = 2.5f;
      profile_gain_ = 3.f;
    }
    const float ratio = std::pow(high / low, 1.f / float(count_ - 1));
    if (q == 0.f) q = std::sqrt(ratio) / (ratio - 1.f);
    band_ratio_octaves_ = std::log2(high / low);
    mod_hpf_.setup(400.f, .7f);
    float hz = low;
    for (int i = 0; i < count_; ++i, hz *= ratio) {
      analysis_.setup(i, hz, q);
      synthesis_.setup(i, hz, q);
      env_[i] = 0.f;
    }
    mapped_formant_ = 1000;
    updateMapping(get(Formant));
  }

  void updateMapping(int semitones) {
    const float shift = float(semitones) / 12.f * float(count_ - 1) /
                        band_ratio_octaves_;
    for (int i = 0; i < count_; ++i) {
      const float position = float(i) - shift;
      int index = int(position);
      if (position < float(index)) --index;
      map_index_[i] = index;
      map_fraction_[i] = position - float(index);
    }
    mapped_formant_ = semitones;
  }

  TimeEffects effects_;
  std::atomic<int> params_[ParamCount];
  BPFBank analysis_, synthesis_;
  HighPass mod_hpf_;
  alignas(16) float env_[16]{};
  alignas(16) float band_gain_[16]{};
  int map_index_[16]{};
  float map_fraction_[16]{};
  static const int count_ = 12;
  int profile_ = 1;
  int mapped_formant_ = 1000;
  bool fault_ = false;
  bool hpf_enabled_ = true;
  float band_ratio_octaves_ = 1.f;
  float env_scale_ = 25.f, env_cap_ = 5.f, carrier_gain_ = 2.5f;
  float profile_gain_ = 1.f;
  float fade_ = 1.f;
  float mix_ = 1.f, modgain_ = 1.f, outgain_ = 1.f;
  float voice_ = 0.f, width_ = 1.f, unvoiced_ = 0.f;
  float attack_ = 0.f, release_ = 0.f;
  float carrier_env_ = 0.f, hp_prev_ = 0.f, hp_state_ = 0.f;
  static const int kChorusBufferSize = 512;
  static const int kMaxChorusDelay = 456;
  float chorus_left_[kChorusBufferSize]{}, chorus_right_[kChorusBufferSize]{};
  int chorus_write_ = 0, chorus_warmup_ = kMaxChorusDelay;
  float chorus_phase_ = 0.f, chorus_ = .35f;
  bool monitor_active_ = false;
};
