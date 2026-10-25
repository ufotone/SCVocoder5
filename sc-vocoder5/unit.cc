#include "unit.h"
#include "vocoder.h"
static Vocoder dsp;
static uint8_t input_channels=4;
__unit_callback int8_t unit_init(const unit_runtime_desc_t* desc) {
  if(!desc) return k_unit_err_undef;
  if(desc->target!=unit_header.target) return k_unit_err_target;
  if(!UNIT_API_IS_COMPAT(desc->api)) return k_unit_err_api_version;
  if(desc->samplerate!=48000) return k_unit_err_samplerate;
  if((desc->input_channels!=4 && desc->input_channels!=2)||desc->output_channels!=2) return k_unit_err_geometry;
  input_channels=desc->input_channels;dsp.reset();return k_unit_err_none;
}
__unit_callback void unit_render(const float* in,float* out,uint32_t frames) {dsp.process(in,out,frames,input_channels);}
__unit_callback void unit_reset() {dsp.reset();}
__unit_callback void unit_resume() {dsp.reset();}
__unit_callback void unit_suspend() {}
__unit_callback void unit_teardown() {}
__unit_callback void unit_set_param_value(uint8_t id,int32_t value) {dsp.set(id,value);}
__unit_callback int32_t unit_get_param_value(uint8_t id) {return dsp.get(id);}
__unit_callback const char* unit_get_param_str_value(uint8_t id,int32_t value) {
  if(id==Vocoder::Profile) {static const char* s[]={"FULL","AGE"};return value>=0&&value<2?s[value]:"?";}
  if(id==Vocoder::InputMode) return value==0?"SIDECHAIN":"L/R SPLIT";
  if(id==Vocoder::Monitor) {static const char* s[]={"VOCODER","CARRIER","MODULATOR"};return value>=0&&value<3?s[value]:"?";}
  return "";
}
__unit_callback uint8_t unit_get_preset_index() {return 0;}
__unit_callback const char* unit_get_preset_name(uint8_t) {return "";}
__unit_callback void unit_load_preset(uint8_t) {}
__unit_callback void unit_set_tempo(uint32_t) {}
