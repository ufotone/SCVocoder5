#include "unit.h"
#include "../vocoder.h"
#include <cstdio>
#include <cstdlib>
static void check(bool ok) {if(!ok)std::exit(1);}
int main() {
 unit_runtime_desc_t d={};d.target=unit_header.target;d.api=UNIT_API_VERSION;d.samplerate=48000;d.input_channels=4;d.output_channels=2;
 check(unit_init(nullptr)==k_unit_err_undef);
 check(unit_init(&d)==0);check(unit_header.num_params==Vocoder::ParamCount);
 for(int p=0;p<Vocoder::ParamCount;++p){check(unit_header.params[p].min==Vocoder::minimum(p));check(unit_header.params[p].max==Vocoder::maximum(p));check(unit_header.params[p].init==Vocoder::initial(p));check(unit_get_param_value((uint8_t)p)==Vocoder::initial(p));}
 d.samplerate=44100;check(unit_init(&d)==k_unit_err_samplerate);d.samplerate=48000;
 d.input_channels=3;check(unit_init(&d)==k_unit_err_geometry);d.input_channels=2;check(unit_init(&d)==0);
 d.target=0;check(unit_init(&d)==k_unit_err_target);d.target=unit_header.target;
 d.api=0;check(unit_init(&d)==k_unit_err_api_version);d.api=UNIT_API_VERSION;
 d.input_channels=4;check(unit_init(&d)==0);
 unit_set_param_value(Vocoder::Monitor,2);unit_set_param_value(Vocoder::ModGain,0);unit_set_param_value(Vocoder::OutputGain,0);unit_resume();
 float input[]={.1f,.2f,.3f,.5f},output[2]={};unit_render(input,output,1);check(std::fabs(output[0]-.4f)<1e-6f);check(output[0]==output[1]);
 unit_reset();check(unit_get_param_value(Vocoder::Monitor)==2);
 std::puts("PASS: SDK init errors, geometry, metadata/defaults, parameter callbacks, stereo sidechain render, reset persistence (host ABI only).");
}
