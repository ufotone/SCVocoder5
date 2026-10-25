#include "unit.h"
// Experimental local ID 'U4TN'; check/register uniqueness before public release.
const __unit_header unit_header_t unit_header = {
 .header_size=sizeof(unit_header_t),
 .target=UNIT_TARGET_PLATFORM | k_unit_module_masterfx,
 .api=UNIT_API_VERSION,.dev_id=0x5534544EU,.unit_id=0x564F4335U,
 .version=0x00000501U,.name="SC Vocoder 5",.num_presets=0,.num_params=13,
 .params={
  {-12,12,0,0,k_unit_param_type_semi,0,0,0,{"FORMANT"}},
  {1,500,1,100,k_unit_param_type_msec,0,0,0,{"RELEASE"}},
  {0,1,0,1,k_unit_param_type_strings,0,0,0,{"RANGE"}},
  {0,100,0,100,k_unit_param_type_percent,0,0,0,{"MIX"}},
  {1,50,1,4,k_unit_param_type_msec,0,0,0,{"ATTACK"}},
  {-24,50,0,35,k_unit_param_type_db,0,0,0,{"MOD GAIN"}},
  {-24,12,0,0,k_unit_param_type_db,0,0,0,{"OUTPUT"}},
  {0,100,0,0,k_unit_param_type_percent,0,0,0,{"UNVOICED"}},
  {0,1,0,0,k_unit_param_type_strings,0,0,0,{"INPUT"}},
  {0,2,0,0,k_unit_param_type_strings,0,0,0,{"MONITOR"}},
  {0,100,0,0,k_unit_param_type_percent,0,0,0,{"VOICE DRY"}},
  {0,100,0,100,k_unit_param_type_percent,0,0,0,{"WIDTH"}},
  {0,100,0,35,k_unit_param_type_percent,0,0,0,{"CHORUS"}},
  {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}
 }
};
