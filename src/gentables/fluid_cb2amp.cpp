
#include "utils/fluid_conv_tables.h"
#include "fluidsynth_priv.h"
#include "auto_gen_math.h"

extern "C" const fluid_real_t fluid_cb2amp_tab[] =
{
/* centibels to amplitude conversion
 * Note: SF2.01 section 8.1.3: Initial attenuation range is
 * between 0 and 144 dB. Therefore a negative attenuation is
 * not allowed.
 */
#define X(i) (fluid_real_t)EXP10(static_cast<fluid_real_t>(i) / -200.0),
#define AUTO_GEN_ARRAY_SIZE FLUID_CB_AMP_SIZE
#include "auto_gen_array.h"
};
