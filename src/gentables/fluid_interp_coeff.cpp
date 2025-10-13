
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "fluidsynth_priv.h"

extern "C" const fluid_real_t interp_coeff[] =
{
//for (i = 0; i < FLUID_INTERP_MAX; i++)
//{
//    x = (double)i / (double)FLUID_INTERP_MAX;

//    interp_coeff[i][0] = (x * (-0.5 + x * (1 - 0.5 * x)));
//    interp_coeff[i][1] = (1.0 + x * x * (1.5 * x - 2.5));
//    interp_coeff[i][2] = (x * (0.5 + x * (2.0 - 1.5 * x)));
//    interp_coeff[i][3] = (0.5 * x * x * (x - 1.0));
//}
#define x(i) ((double)(i) / (double)FLUID_INTERP_MAX)
#define X(i) \
    (fluid_real_t)(x(i) * (-0.5 + x(i) * (1 - 0.5 * x(i)))), \
    (fluid_real_t)(1.0 + x(i) * x(i) * (1.5 * x(i) - 2.5)), \
    (fluid_real_t)(x(i) * (0.5 + x(i) * (2.0 - 1.5 * x(i)))), \
    (fluid_real_t)(0.5 * x(i) * x(i) * (x(i) - 1.0)),
#define AUTO_GEN_ARRAY_SIZE FLUID_INTERP_MAX
#include "auto_gen_array.h"
};
