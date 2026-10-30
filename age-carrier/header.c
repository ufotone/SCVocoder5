#include "unit.h"

// Experimental local developer ID 'U4TN' and unit ID 'ACR1'.
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t),
 .target=UNIT_TARGET_PLATFORM | k_unit_module_synth,
 .api=UNIT_API_VERSION,.dev_id=0x5534544EU,.unit_id=0x41435231U,
 .version=0x00000400U,.name="AGE Carrier",.num_presets=0,.num_params=9,
 .params={
  {1,200,1,5,k_unit_param_type_msec,0,0,0,{"ATTACK"}},
  {1,1000,1,80,k_unit_param_type_msec,0,0,0,{"DECAY"}},
  {5,2000,5,120,k_unit_param_type_msec,0,0,0,{"RELEASE"}},
  {-24,0,0,0,k_unit_param_type_db,0,0,0,{"LEVEL"}},
  {0,39,0,0,k_unit_param_type_strings,0,0,0,{"CHORD"}},
  {0,11,0,0,k_unit_param_type_strings,0,0,0,{"ROOT"}},
  {1,6,1,3,k_unit_param_type_none,0,0,0,{"OCTAVE"}},
  {0,2,0,0,k_unit_param_type_strings,0,0,0,{"MODE"}},
  {0,12,0,0,k_unit_param_type_strings,0,0,0,{"BASS"}},
  {0},{0},{0},{0},{0},{0},{0},
  {0},{0},{0},{0},{0},{0},{0},{0}
 }
};
