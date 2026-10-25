#include "../synth.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

static void check(bool condition, const char* name) {
  if (!condition) { std::printf("FAIL: %s\n", name); std::exit(1); }
}

static double energy(const std::vector<float>& signal) {
  double result = 0.0;
  for (float value : signal) {
    check(std::isfinite(value) && std::fabs(value) <= 1.f, "finite bounded output");
    result += value * value;
  }
  return result / signal.size();
}

int main() {
  const int frames = 48000;
  std::vector<float> output(frames * 2), whole(frames * 2), chunked(frames * 2);

  unit_runtime_desc_t descriptor = {};
  descriptor.samplerate = 48000;
  descriptor.output_channels = 2;

  Synth synth;
  check(synth.Init(&descriptor) == 0, "init");
  synth.NoteOn(60, 110); synth.NoteOn(64, 110); synth.NoteOn(67, 110);
  synth.Render(output.data(), frames);
  const double chordEnergy = energy(output);
  check(chordEnergy > 1e-5, "C major chord output");
  for (int i = 0; i < frames; ++i) check(output[i * 2] == output[i * 2 + 1], "dual mono output");

  synth.NoteOff(60); synth.NoteOff(64); synth.NoteOff(67);
  synth.Render(output.data(), frames);
  double tail = 0.0;
  for (int i = frames - 4800; i < frames; ++i) tail += output[i * 2] * output[i * 2];
  check(tail < 1e-8, "release reaches silence");

  Synth a, b;
  a.NoteOn(60, 100); a.NoteOn(64, 100); a.NoteOn(67, 100);
  b.NoteOn(60, 100); b.NoteOn(64, 100); b.NoteOn(67, 100);
  a.Render(whole.data(), frames);
  for (int offset = 0; offset < frames;) {
    const int count = std::min(37, frames - offset);
    b.Render(chunked.data() + offset * 2, count);
    offset += count;
  }
  check(whole == chunked, "block size independence");

  for (int parameter = 0; parameter < Synth::ParamCount; ++parameter) {
    synth.setParameter(uint8_t(parameter), 100000);
    check(synth.getParameterValue(uint8_t(parameter)) == Synth::maximum(parameter), "upper parameter bound");
    synth.setParameter(uint8_t(parameter), -100000);
    check(synth.getParameterValue(uint8_t(parameter)) == Synth::minimum(parameter), "lower parameter bound");
    synth.setParameter(uint8_t(parameter), Synth::initial(parameter));
  }

  synth.Reset();
  for (int note = 48; note < 60; ++note) synth.NoteOn(uint8_t(note), 127);
  synth.Render(output.data(), frames);
  check(energy(output) > 1e-5, "six voices and stealing");
  synth.AllNoteOff();
  synth.Render(output.data(), 512);
  double stopped = 0.0;
  for (int i = 0; i < 1024; ++i) stopped += output[i] * output[i];
  check(stopped == 0.0, "all note off is immediate");

  synth.Reset();
  for (int note = 48; note < 54; ++note) synth.NoteOn(uint8_t(note), 110);
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < 10; ++i) synth.Render(output.data(), frames);
  const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  std::printf("PASS: chord, ADR, mono, block independence, bounds, voice stealing, all-note-off.\n");
  std::printf("Chord RMS %.6f; 10 seconds / 6 voices in %.3fs on host (not ARM benchmark).\n",
              std::sqrt(chordEnergy), elapsed);
}
