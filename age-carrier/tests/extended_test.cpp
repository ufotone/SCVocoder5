#include "../synth.h"
#define Synth ReferenceSynth
#include "age_reference.h"
#undef Synth
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void check(bool ok,const char* message){if(!ok){std::printf("FAIL: %s\n",message);std::exit(1);}}
static void compare(Synth& a,ReferenceSynth& b,int frames=2048){
 std::vector<float>x(frames*2),y(frames*2);a.Render(x.data(),frames);b.Render(y.data(),frames);
 for(int i=0;i<frames*2;++i)check(std::isfinite(x[i])&&std::fabs(x[i]-y[i])<1e-5f,"generated chord waveform equals manual AGE voices");
}
static void explicitChord(int type,int root,int bass,std::initializer_list<int> notes){
 Synth a;ReferenceSynth b;a.setParameter(Synth::Mode,2);a.setParameter(Synth::Root,root);a.setParameter(Synth::Chord,type);a.setParameter(Synth::Bass,bass);a.NoteOn(60,100);
 for(int n:notes)b.NoteOn(uint8_t(n),100);compare(a,b);a.PitchBend(10000);b.PitchBend(10000);compare(a,b);
 a.NoteOff(60);for(int n:notes)b.NoteOff(uint8_t(n));compare(a,b);
}
int main(){
 explicitChord(8,7,0,{55,58,62,65}); // Gm7: G Bb D F
 explicitChord(10,7,0,{55,58,62,64}); // Gm6: G Bb D E
 explicitChord(11,7,0,{55,58,62,63}); // Gm(b6): G Bb D Eb
 explicitChord(7,8,8,{55,56,60,63}); // AbM7/G: G Ab C Eb
 explicitChord(7,8,0,{56,60,63,67}); // AbM7 without a bass override
 explicitChord(18,0,2,{37,52,59,62}); // Cmaj9/Db: rootless, four voices including bass
 explicitChord(0,0,8,{43,48,52}); // C/G inversion, not a duplicated fifth
 explicitChord(8,7,8,{55,58,62,65}); // Explicit root bass preserves Gm7
 explicitChord(17,0,0,{48,52,58,62}); // C9: omit the fifth
 explicitChord(18,0,0,{48,52,59,62}); // Cmaj9: omit the fifth
 explicitChord(19,0,0,{48,51,58,62}); // Cm9: omit the fifth
 explicitChord(21,0,0,{48,52,57,62}); // C6/9: omit the fifth
 explicitChord(6,0,2,{37,48,52,58}); // C7/Db: omit natural fifth to fit bass
 explicitChord(24,0,2,{37,52,56,58}); // C7#5/Db: keep altered fifth, omit root
 const int counts[40]={3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
 const int notes[40][4]={{0,4,7},{0,3,7},{0,3,6},{0,4,8},{0,2,7},{0,5,7},{0,4,7,10},{0,4,7,11},{0,3,7,10},{0,3,6,10},{0,3,7,9},{0,3,7,8},{0,4,7,9},{0,3,6,9},{0,3,7,11},{0,4,7,14},{0,3,7,14},{0,4,10,14},{0,4,11,14},{0,3,10,14},{0,5,7,10},{0,4,9,14},{0,3,9,14},{0,4,6,10},{0,4,8,10},{0,4,8,11},{0,4,10,13},{0,4,10,15},{0,4,10,18},{0,4,10,20},{0,4,10,21},{0,4,11,21},{0,3,10,21},{0,10,14,17},{0,3,10,17},{0,2,7,10},{0,2,7,11},{0,5,7,11},{0,4,7,17},{0,3,7,17}};
 for(int mode:{1,2})for(int chord=0;chord<40;++chord)for(int root=0;root<12;++root)for(int bass=0;bass<=12;++bass){
   Synth a;ReferenceSynth b;a.setParameter(Synth::Mode,mode);a.setParameter(Synth::Chord,chord);a.setParameter(Synth::Root,root);a.setParameter(Synth::Bass,bass);a.NoteOn(uint8_t(60+root),100);
   const int base=(mode==1?60:48)+root;std::vector<int> expected;
   if(bass>0)expected.push_back(base-((base%12-(bass-1)+12)%12));
   for(int i=0;i<counts[chord];++i){int note=base+notes[chord][i];if(bass==0||note%12!=bass-1)expected.push_back(note);}
   if(expected.size()>4){
     auto remove=std::find(expected.begin(),expected.end(),base+7);
     if(remove==expected.end())remove=std::find(expected.begin(),expected.end(),base);
     check(remove!=expected.end(),"an omittable chord tone exists");expected.erase(remove);
   }
   check(expected.size()<=4,"generated voicing including bass fits four voices");
   for(int n:expected)b.NoteOn(uint8_t(n),100);compare(a,b,512);
 }
 // Full MIDI range, including cases where a low slash bass requires lifting the root.
 for(int trigger=0;trigger<128;++trigger)for(int chord=0;chord<40;++chord)for(int bass=0;bass<=12;++bass){
   Synth a;a.setParameter(Synth::Mode,1);a.setParameter(Synth::Chord,chord);a.setParameter(Synth::Bass,bass);a.NoteOn(uint8_t(trigger),127);float x[128];a.Render(x,64);for(float v:x)check(std::isfinite(v)&&std::fabs(v)<=1.f,"all MIDI slash/extension combinations stay bounded");
 }
 Synth labels;check(std::strcmp(labels.getParameterStrValue(Synth::Chord,10),"MIN6")==0,"MIN6 label");check(std::strcmp(labels.getParameterStrValue(Synth::Chord,11),"MINb6")==0,"minor flat six label");check(std::strcmp(labels.getParameterStrValue(Synth::Root,8),"Ab")==0,"Ab root label");check(std::strcmp(labels.getParameterStrValue(Synth::Bass,8),"G")==0,"G bass label");check(std::strcmp(labels.getParameterStrValue(Synth::Bass,0),"OFF")==0,"bass off label");
 // LIVE6 ignores chord-generation controls, including bass.
 Synth live;ReferenceSynth original;live.setParameter(Synth::Bass,12);live.setParameter(Synth::Chord,22);for(int n:{48,52,55,59,62,65}){live.NoteOn(uint8_t(n),100);original.NoteOn(uint8_t(n),100);}compare(live,original);
 std::puts("PASS: requested progression, inversions, four-note extensions, 12480 generated voicings versus manual AGE voices, full MIDI/bass/type bounds, labels, LIVE6 preservation.");
}
