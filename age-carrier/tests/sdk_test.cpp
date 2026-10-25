#include "unit.h"
#include "../synth.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

static void check(bool condition) { if (!condition) std::exit(1); }

int main() {
  unit_runtime_desc_t descriptor = {};
  descriptor.target = unit_header.target;
  descriptor.api = UNIT_API_VERSION;
  descriptor.samplerate = 48000;
  descriptor.output_channels = 2;
  check(unit_init(nullptr) == k_unit_err_undef);
  check(unit_init(&descriptor) == 0);
  check(unit_header.num_params == Synth::ParamCount);
  for (int parameter = 0; parameter < Synth::ParamCount; ++parameter) {
    check(unit_header.params[parameter].min == Synth::minimum(parameter));
    check(unit_header.params[parameter].max == Synth::maximum(parameter));
    check(unit_header.params[parameter].init == Synth::initial(parameter));
    check(unit_get_param_value(uint8_t(parameter)) == Synth::initial(parameter));
  }
  descriptor.samplerate = 44100;
  check(unit_init(&descriptor) == k_unit_err_samplerate);
  descriptor.samplerate = 48000;
  descriptor.output_channels = 1;
  check(unit_init(&descriptor) == k_unit_err_geometry);
  descriptor.output_channels = 2;
  check(unit_init(&descriptor) == 0);
  unit_note_on(60, 127); unit_note_on(64, 127); unit_note_on(67, 127);
  float output[256] = {};
  unit_render(nullptr, output, 128);
  float sum = 0.f;
  for (float value : output) { check(std::isfinite(value)); sum += value * value; }
  check(sum > 0.f);
  unit_all_note_off();
  unit_render(nullptr, output, 128);
  for (float value : output) check(value == 0.f);
  std::puts("PASS: SDK init, metadata, parameter callbacks, chord note callbacks, render, all-note-off.");
}
