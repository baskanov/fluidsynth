
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "fluidsynth_priv.h"

#include "gcem.hpp"

extern "C" const fluid_real_t sinc_table7[] =
{
// Below is the original code. Note how indexing of sinc_table7 does not happen sequentially in this version:
//
///* i: Offset in terms of whole samples */
//for (i = 0; i < SINC_INTERP_ORDER; i++)
//{
//    /* i2: Offset in terms of fractional samples ('subsamples') */
//    for (i2 = 0; i2 < FLUID_INTERP_MAX; i2++)
//    {
//        /* center on middle of table */
//        i_shifted = (double)i - ((double)SINC_INTERP_ORDER / 2.0) +
//                    (double)i2 / (double)FLUID_INTERP_MAX;

//        /* sinc(0) cannot be calculated straightforward (limit needed for 0/0) */
//        if (fabs(i_shifted) > 0.000001)
//        {
//            double arg = M_PI * i_shifted;
//            v = sin(arg) / (arg);
//            /* Hanning window */
//            v *= 0.5 * (1.0 + cos(2.0 * arg / (double)SINC_INTERP_ORDER));
//        }
//        else
//        {
//            v = 1.0;
//        }

//        sinc_table7[FLUID_INTERP_MAX - i2 - 1][i] = v;
//    }
//}
//
// To make the indexing happen sequentially, we transform it into this:
//
//for (i2 = 0; i2 < FLUID_INTERP_MAX; i2++)
//{
//    for (i = 0; i < SINC_INTERP_ORDER; i++)
//    {
//        /* Compute the shifted index as in the original code */
//        double i_shifted = (double)i - ((double)SINC_INTERP_ORDER / 2.0) +
//                           (double)(FLUID_INTERP_MAX - i2 - 1) / (double)FLUID_INTERP_MAX;

//        if (fabs(i_shifted) > 0.000001)
//        {
//            double arg = M_PI * i_shifted;
//            v = sin(arg) / arg;
//            /* Hanning window */
//            v *= 0.5 * (1.0 + cos(2.0 * arg / (double)SINC_INTERP_ORDER));
//        }
//        else
//        {
//            v = 1.0;
//        }

//        sinc_table7[i2][i] = v;
//    }
//}
#define I_SHIFTED(i, i2) ((fluid_real_t)(i) - ((fluid_real_t)SINC_INTERP_ORDER / 2.0) + (fluid_real_t)(FLUID_INTERP_MAX - (i2) - 1) / (fluid_real_t)FLUID_INTERP_MAX)
#define ARG(i, i2) (static_cast<double>(GCEM_PI) * I_SHIFTED(i, i2))
#define SINC_TABLE(i, i2) (gcem::fabs(I_SHIFTED(i, i2)) > 0.000001 \
        ? (gcem::sin(ARG(i, i2)) / (ARG(i, i2))) * (0.5 * (1.0 + gcem::cos(2.0 * ARG(i, i2) / (fluid_real_t)SINC_INTERP_ORDER))) \
        : 1.0)
#define X(i) \
    (fluid_real_t)SINC_TABLE(0, i), \
    (fluid_real_t)SINC_TABLE(1, i), \
    (fluid_real_t)SINC_TABLE(2, i), \
    (fluid_real_t)SINC_TABLE(3, i), \
    (fluid_real_t)SINC_TABLE(4, i), \
    (fluid_real_t)SINC_TABLE(5, i), \
    (fluid_real_t)SINC_TABLE(6, i),
#define AUTO_GEN_ARRAY_SIZE FLUID_INTERP_MAX
#include "auto_gen_array.h"
};
