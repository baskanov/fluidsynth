
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "fluidsynth_priv.h"

#include "gcem.hpp"

extern "C" const fluid_real_t interp_coeff_linear[] =
{
//for (i = 0; i < FLUID_INTERP_MAX; i++)
//{
//    x = (double)i / (double)FLUID_INTERP_MAX;
//    interp_coeff_linear[i][0] = (1.0 - x);
//    interp_coeff_linear[i][1] = x;
//}
#define x(i) ((double)(i) / (double)FLUID_INTERP_MAX)
#define X(i) (fluid_real_t)(1.0 - x(i)), (fluid_real_t)x(i),
#define AUTO_GEN_ARRAY_SIZE FLUID_INTERP_MAX
#include "auto_gen_array.h"
};
