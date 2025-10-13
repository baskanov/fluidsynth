
#include "utils/fluid_conv_tables.h"
#include "fluidsynth_priv.h"
#include "auto_gen_math.h"

extern "C" const fluid_real_t fluid_pan_tab[] =
{
/* initialize the pan conversion table */
#define X(i) static_cast<fluid_real_t>(SINPI((i) * (0.5 / (FLUID_PAN_SIZE - 1.0)))),
#define AUTO_GEN_ARRAY_SIZE FLUID_PAN_SIZE
#include "auto_gen_array.h"
};
