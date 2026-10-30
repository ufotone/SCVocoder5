#include "../vocoder.h"
#define Vocoder ReferenceVocoder
#include "vocoder_v051_reference.h"
#undef Vocoder
#include <vector>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <chrono>
static void check(bool ok,const char* message){if(!ok){std::printf("FAIL: %s\n",message);std::exit(1);}}
static void bounded(const std::vector<float>& x){for(float v:x)check(std::isfinite(v)&&std::fabs(v)<=1.f,"finite bounded audio");}
static double energy(const std::vector<float>& x,int start,int end){double s=0;for(int i=start;i<end;++i)s+=double(x[i])*x[i];return s;}
int main(){
 const int n=96000;std::vector<float> in(n*4),out(n*2),ref(n*2),chunk(n*2);
 for(int i=0;i<n;++i){float t=i/48000.f;in[i*4]=in[i*4+1]=.12f*std::sin(1382.3008f*t)+.06f*std::sin(5529.203f*t);in[i*4+2]=in[i*4+3]=.15f*std::sin(5529.203f*t);}
 auto v=std::unique_ptr<Vocoder>(new Vocoder);auto old=std::unique_ptr<ReferenceVocoder>(new ReferenceVocoder);
 v->process(in.data(),out.data(),n,4);old->process(in.data(),ref.data(),n,4);check(out==ref,"new effects OFF exactly matches v0.5.1");
 for(int p:{Vocoder::Flanger,Vocoder::Reverb}){
   for(int i=0;i<Vocoder::ParamCount;++i)v->set(i,Vocoder::initial(i));
   v->set(p,75);v->reset();v->process(in.data(),out.data(),n,4);bounded(out);
   double diff=0;for(int i=0;i<n*2;++i){double d=out[i]-ref[i];diff+=d*d;}check(diff>1e-6,"each effect changes wet signal");
 }
 v->set(Vocoder::Flanger,70);v->set(Vocoder::Reverb,65);v->reset();v->process(in.data(),out.data(),n,4);v->reset();
 for(int i=0;i<n;){int frames=std::min(37,n-i);v->process(in.data()+i*4,chunk.data()+i*2,frames,4);i+=frames;}check(out==chunk,"all FX block size independent");
 v->set(Vocoder::Mix,0);v->reset();v->process(in.data(),out.data(),n,4);for(int i=0;i<n;++i)check(out[i*2]==in[i*4]&&out[i*2+1]==in[i*4+1],"MIX zero retains dry signal with all FX");
 for(int mode:{1,2}){v->set(Vocoder::Monitor,mode);v->set(Vocoder::ModGain,0);v->reset();v->process(in.data(),out.data(),n,4);for(int i=0;i<n;++i)check(out[i*2]==in[i*4+(mode==1?0:2)],"monitor bypasses all FX");}
 v->set(Vocoder::Monitor,0);v->set(Vocoder::Mix,100);std::fill(in.begin(),in.end(),0.f);v->process(in.data(),out.data(),n,4);for(float x:out)check(x==0.f,"no old FX tail when leaving monitor");
 auto fx=std::unique_ptr<TimeEffects>(new TimeEffects);
 // Broadband RMS check for the amount-dependent automatic makeup gain.
 for(int amount:{25,50,75,100}){
   fx->prepare(amount,20,0,1800);fx->reset();uint32_t rng=17;double inputEnergy=0,outputEnergy=0;
   for(int i=0;i<144000;++i){rng=rng*1664525U+1013904223U;float x=.15f*(2.f*float(rng>>8)/16777215.f-1.f);float l=x,r=x;fx->tick(l,r);if(i>=48000){inputEnergy+=2.*x*x;outputEnergy+=double(l)*l+double(r)*r;}}
   const double level=10.*std::log10(outputEnergy/inputEnergy);
   std::printf("Flanger %d%% broadband level: %+.2f dB relative to dry\n",amount,level);
   check(level>-.8&&level<1.5,"flanger makeup prevents broadband level loss without excessive boost");
 }
 std::vector<float> shortTail(288000),longTail(288000);
 for(int decay:{300,5000}){
   fx->prepare(0,20,100,decay);fx->reset();auto& tail=decay==300?shortTail:longTail;
   for(int i=0;i<int(tail.size());++i){float a=i==0?.2f:0.f,b=0.f;fx->tick(a,b);check(std::isfinite(a)&&std::isfinite(b),"reverb finite tail");tail[i]=a+b;}
 }
 check(energy(longTail,4800,48000)>1e-8,"reverb produces a tail");
 check(energy(longTail,48000,96000)>energy(shortTail,48000,96000)*10.,"REV TIME increases tail duration");
 check(energy(longTail,240000,288000)<energy(longTail,4800,52800)*.05,"long reverb tail decays");
 // Both effects must discard frozen audio after they are disabled.
 fx->prepare(100,20,100,1800);fx->reset();for(int i=0;i<2000;++i){float a=.1f,b=0.f;fx->tick(a,b);}
 fx->prepare(0,20,0,1800);for(int i=0;i<5000;++i){float a=0,b=0;fx->tick(a,b);}
 fx->prepare(100,20,100,1800);for(int i=0;i<5000;++i){float a=0,b=0;fx->tick(a,b);check(a==0&&b==0,"no stale FX after re-enable");}
 fx->reset();float silenceL=0,silenceR=0;fx->tick(silenceL,silenceR);check(silenceL==0&&silenceR==0,"reset clears effects");
 check(sizeof(Vocoder)<100000,"dedicated echo buffer was removed");
 check(Vocoder::ParamCount==17,"only the 17 remaining controls are exposed");
 // Maximum time effects together, including live knob changes and silence decay.
 v->set(Vocoder::Monitor,0);for(int p:{Vocoder::Flanger,Vocoder::Reverb,Vocoder::ReverbTime})v->set(p,Vocoder::maximum(p));v->reset();
 for(int i=0;i<n;++i){in[i*4]=in[i*4+1]=.2f*std::sin(i*.07f);in[i*4+2]=in[i*4+3]=.2f*std::sin(i*.11f);}
 auto start=std::chrono::steady_clock::now();for(int i=0;i<5;++i)v->process(in.data(),out.data(),n,4);bounded(out);
 std::printf("All effects: 10s audio processed in %.3fs on host; Vocoder storage %zu bytes (not an ARM benchmark).\n",std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count(),sizeof(Vocoder));
 v->set(Vocoder::ReverbTime,300);v->set(Vocoder::FlangerRate,100);v->process(in.data(),out.data(),n,4);bounded(out);
 std::fill(in.begin(),in.end(),0.f);for(int i=0;i<10;++i)v->process(in.data(),out.data(),n,4);bounded(out);check(energy(out,0,n*2)<1e-6,"combined feedback eventually decays");
 std::puts("PASS: v0.5.1 bypass compatibility, each FX, chunking, dry/monitor bypass, tail reset, reverb decay, flanger makeup RMS, effect re-enable, bounds and live controls.");
}
