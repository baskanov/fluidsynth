
#include "utils/fluid_conv_tables.h"
#include "fluidsynth_priv.h"
#include "auto_gen_math.h"

extern "C" const fluid_real_t fluid_concave_tab[] =
{
/* There seems to be an error in the specs. The equations are
   implemented according to the pictures on SF2.01 page 73. */
#define X(i) (fluid_real_t)(((i) == 0) \
    ? 0 \
    : (((i) == FLUID_VEL_CB_SIZE - 1) \
        ? 1 \
        : ((-200.0 * 2 / FLUID_PEAK_ATTENUATION) * LOG(((FLUID_VEL_CB_SIZE - 1) - (i)) / (FLUID_VEL_CB_SIZE - 1.0)) / static_cast<double>(M_LN10)) \
        )),
#define AUTO_GEN_ARRAY_SIZE FLUID_VEL_CB_SIZE
#include "auto_gen_array.h"
};
