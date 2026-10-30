#include "../synth.h"
#define Synth ReferenceSynth
#include "age_reference.h"
#undef Synth
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void check(bool x, const char* msg) { if(!x){std::printf("FAIL: %s\n",msg);std::exit(1);} }
static void equal(Synth& a, ReferenceSynth& b) {
  std::vector<float> x(4096),y(4096);
  a.Render(x.data(),2048);b.Render(y.data(),2048);
  for(size_t i=0;i<x.size();++i) if(!(std::isfinite(x[i]) && std::fabs(x[i]-y[i])<1e-5f)){ std::printf("difference at %zu: %.8f %.8f\n",i,x[i],y[i]);check(false,"generated waveform matches manual AGE chord");}
}
int main(){
 Synth live;ReferenceSynth original;
 check(live.getParameterValue(Synth::Mode)==0,"default LIVE6");
 for(int n:{48,52,55,59,62,65}){live.NoteOn(uint8_t(n),100);original.NoteOn(uint8_t(n),100);}equal(live,original);
 live.NoteOff(52);original.NoteOff(52);equal(live,original);
 live.NoteOn(70,127);original.NoteOn(70,127);equal(live,original);
 for(int mode:{1,2,0}){live.setParameter(Synth::Mode,mode);float silence[256];live.Render(silence,128);for(float x:silence)check(x==0,"mode switch clears old voices");}
 const int intervals[10][4]={{0,4,7,0},{0,3,7,0},{0,3,6,0},{0,4,8,0},{0,2,7,0},{0,5,7,0},{0,4,7,10},{0,4,7,11},{0,3,7,10},{0,3,6,10}};
 for(int mode=0;mode<2;++mode) for(int chord=0;chord<10;++chord) for(int root=0;root<12;++root){
   Synth a; ReferenceSynth b;
   a.setParameter(Synth::Mode,mode?1:2);a.setParameter(Synth::Chord,chord);a.setParameter(Synth::Root,root);
   a.NoteOn(uint8_t(60+root),100);
   int base=(mode?60:48)+root;
   for(int i=0;i<(chord>=6?4:3);++i)b.NoteOn(uint8_t(base+intervals[chord][i]),100);
   equal(a,b);a.PitchBend(12000);b.PitchBend(12000);equal(a,b);
   a.NoteOff(uint8_t(60+root));for(int i=0;i<(chord>=6?4:3);++i)b.NoteOff(uint8_t(base+intervals[chord][i]));equal(a,b);
 }
 std::puts("PASS: 240 chord waveforms, bend and release");
 // Fourth voice must disappear on a change from seventh to triad.
 Synth a;ReferenceSynth b;a.setParameter(Synth::Mode,2);a.setParameter(Synth::Chord,7);a.NoteOn(70,127);
 float warm[128];a.Render(warm,64);b.Render(warm,64);
 a.setParameter(Synth::Chord,1);a.NoteOn(60,127);for(int n:{48,51,55})b.NoteOn(uint8_t(n),127);
 a.NoteOff(70);
 // Prior chord's DC blocker tail is expected; let it settle in both renders.
 std::vector<float> settle(8192);a.Render(settle.data(),4096);b.Render(settle.data(),4096);equal(a,b);
 a.NoteOn(60,0);for(int n:{48,51,55})b.NoteOff(uint8_t(n));equal(a,b);
 a.AllNoteOff();a.GateOn(127);a.GateOff();std::vector<float> tail(48000);a.Render(tail.data(),24000);
 for(size_t i=40000;i<tail.size();++i)check(std::fabs(tail[i])<1e-8f,"gate releases entire chord");
 a.AllNoteOff();a.Render(tail.data(),24000);for(float x:tail)check(x==0,"panic clears output");
 for(int chord=0;chord<10;++chord){a.setParameter(Synth::Mode,1);a.setParameter(Synth::Chord,chord);a.NoteOn(127,127);a.Render(tail.data(),24000);for(float x:tail)check(std::isfinite(x)&&std::fabs(x)<=1,"top MIDI note remains bounded");}
 for(int i=0;i<Synth::ParamCount;++i){a.setParameter(uint8_t(i),-999);check(a.getParameterValue(uint8_t(i))==Synth::minimum(i),"min clamp");a.setParameter(uint8_t(i),9999);check(a.getParameterValue(uint8_t(i))==Synth::maximum(i),"max clamp");}
 check(std::strcmp(a.getParameterStrValue(Synth::Chord,7),"MAJ7")==0,"chord label");
 std::puts("PASS: 240 generated chords match AGE reference, pitch bend, release, stale note-off, seventh-to-triad, gate, panic, MIDI bounds, parameter bounds, labels.");
}
