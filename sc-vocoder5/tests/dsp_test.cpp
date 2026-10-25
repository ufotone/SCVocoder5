#include "../vocoder.h"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <limits>
static void check(bool ok,const char* name) {if(!ok){std::printf("FAIL: %s\n",name);std::exit(1);}}
static double energy(const std::vector<float>& x) {double s=0;for(float v:x){check(std::isfinite(v)&&std::fabs(v)<=1.f,"finite bounded output");s+=v*v;}return s/x.size();}
int main() {
 const int n=48000;std::vector<float> in(n*4),out(n*2);
 for(int i=0;i<n;++i){float t=float(i)/48000;in[i*4]=.2f*std::sin(6.2831853f*220*t)+.1f*std::sin(6.2831853f*880*t);in[i*4+1]=in[i*4];in[i*4+2]=in[i*4+3]=.3f*std::sin(6.2831853f*880*t);}
 Vocoder v;v.process(in.data(),out.data(),n,4);const double wet=energy(out);check(wet>1e-7,"nonzero vocoder output");
 auto original=in;
 for(int i=0;i<n;++i)in[i*4+2]=in[i*4+3]=0;
 v.reset();v.process(in.data(),out.data(),n,4);check(energy(out)==0,"silent modulator silences wet");
 in=original;for(int i=0;i<n;++i)in[i*4]=in[i*4+1]=0;
 v.reset();v.process(in.data(),out.data(),n,4);check(energy(out)==0,"silent carrier silences wet");
 in=original;v.set(Vocoder::Mix,0);v.reset();v.process(in.data(),out.data(),n,4);
 for(int i=0;i<n;++i)check(std::fabs(out[i*2]-in[i*4])<1e-6,"dry unity");
 v.set(Vocoder::Mix,100);v.set(Vocoder::ModGain,0);v.set(Vocoder::Monitor,2);v.reset();v.process(in.data(),out.data(),n,4);
 for(int i=0;i<n;++i)check(std::fabs(out[i*2]-in[i*4+2])<1e-6,"sidechain indexing");
 v.set(Vocoder::Monitor,0);v.reset();std::vector<float> whole(n*2),chunked(n*2);
 v.process(in.data(),whole.data(),n,4);v.reset();
 for(int i=0;i<n;){int size=std::min(37,n-i);v.process(in.data()+i*4,chunked.data()+i*2,size,4);i+=size;}
 check(whole==chunked,"block size independence");
 // The dedicated chorus must change only the vocoder path and stay deterministic.
 std::vector<float> noChorus(n*2),fullChorus(n*2);
 for(int p=0;p<Vocoder::ParamCount;++p)v.set(p,Vocoder::initial(p));
 v.set(Vocoder::Chorus,0);v.reset();v.process(in.data(),noChorus.data(),n,4);
 v.set(Vocoder::Chorus,100);v.reset();v.process(in.data(),fullChorus.data(),n,4);
 double chorusDifference=0;for(int i=0;i<n*2;++i){const double d=fullChorus[i]-noChorus[i];chorusDifference+=d*d;}
 check(chorusDifference>1e-6,"chorus changes vocoder output");energy(fullChorus);
 v.set(Vocoder::Chorus,Vocoder::initial(Vocoder::Chorus));
 // L/R split must agree with mono-carrier sidechain mode.
 v.reset();v.process(in.data(),whole.data(),n,4);
 std::vector<float> split(n*2);for(int i=0;i<n;++i){split[i*2]=in[i*4];split[i*2+1]=in[i*4+2];}
 v.set(Vocoder::InputMode,1);v.reset();v.process(split.data(),chunked.data(),n,2);
 check(whole==chunked,"split routing equivalence");
 v.set(Vocoder::InputMode,0);v.reset();v.process(split.data(),chunked.data(),n,2);check(energy(chunked)==0,"missing sidechain safely silent");
 // Monitor modes must bypass every filter and leave DSP state untouched.
 in=original;Vocoder monitored,fresh;monitored.set(Vocoder::Monitor,1);monitored.process(in.data(),out.data(),n,4);
 monitored.set(Vocoder::Monitor,0);monitored.process(in.data(),whole.data(),n,4);fresh.process(in.data(),chunked.data(),n,4);
 check(whole==chunked,"monitor is a state-preserving DSP bypass");
 v.set(Vocoder::InputMode,0);
 double profileRms[2]={};
 for(int profile=0;profile<2;++profile)for(int shift=-12;shift<=12;shift+=12){v.set(Vocoder::Profile,profile);v.set(Vocoder::Formant,shift);v.reset();v.process(in.data(),out.data(),n,4);double e=energy(out);check(e>1e-9,"profiles and formant produce audio");if(shift==0)profileRms[profile]=std::sqrt(e);}
 check(profileRms[1]>.5*profileRms[0]&&profileRms[1]<2.*profileRms[0],"profile reference-level matching");
 v.set(Vocoder::Profile,0);v.reset();v.process(in.data(),out.data(),n,4);v.set(Vocoder::Profile,1);v.process(in.data(),out.data(),n,4);energy(out);
 // Extreme controls and alternating bounded pseudo-random inputs.
 for(int p=0;p<Vocoder::ParamCount;++p){v.set(p,100000);check(v.get(p)==Vocoder::maximum(p),"upper bound");v.set(p,-100000);check(v.get(p)==Vocoder::minimum(p),"lower bound");v.set(p,Vocoder::maximum(p));}
 v.set(Vocoder::InputMode,0);v.set(Vocoder::Monitor,0);v.reset();
 uint32_t rng=17;for(float& x:in){rng=rng*1664525U+1013904223U;x=2.f*(float(rng>>8)/16777215.f)-1.f;}
 v.process(in.data(),out.data(),n,4);energy(out);
 // Non-finite host input must never reach the output or poison later blocks.
 in=original;in[0]=std::numeric_limits<float>::infinity();in[2]=std::numeric_limits<float>::quiet_NaN();
 v.reset();v.process(in.data(),out.data(),n,4);energy(out);v.process(original.data(),out.data(),n,4);check(energy(out)>1e-9,"non-finite input recovery");
 // Synthetic release measurement: same carrier, modulator removed after warm-up.
 for(int p=0;p<Vocoder::ParamCount;++p)v.set(p,Vocoder::initial(p));
 in=original;v.reset();v.process(in.data(),out.data(),n,4);
 for(int i=0;i<n;++i)in[i*4+2]=in[i*4+3]=0;
 v.process(in.data(),out.data(),n,4);double early=0,late=0;
 for(int i=0;i<4800;++i){early+=out[i*2]*out[i*2];late+=out[(n-4800+i)*2]*out[(n-4800+i)*2];}
 check(late<early*.001,"envelope release decays");
 double elapsed[2]={};
 for(int profile=0;profile<2;++profile){v.set(Vocoder::Monitor,0);v.set(Vocoder::Profile,profile);v.reset();auto start=std::chrono::steady_clock::now();for(int i=0;i<10;++i)v.process(original.data(),out.data(),n,4);elapsed[profile]=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();}
 v.set(Vocoder::Monitor,1);auto start=std::chrono::steady_clock::now();for(int i=0;i<10;++i)v.process(original.data(),out.data(),n,4);
 double monitorElapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 std::printf("PASS: silence, dry unity, routing, monitor bypass, geometry fallback, chunking, chorus, profiles, formant, transitions, bounds, stress, finite guards, release.\nInitial wet RMS %.6f; profile RMS FULL %.6f, AGE %.6f.\n10 seconds DSP: FULL %.3fs, AGE %.3fs; monitor %.3fs on host (not ARM benchmark).\n",std::sqrt(wet),profileRms[0],profileRms[1],elapsed[0],elapsed[1],monitorElapsed);
}
