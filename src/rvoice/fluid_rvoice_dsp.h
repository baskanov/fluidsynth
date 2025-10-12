#ifndef _FLUID_RVOICE_DSP_H
#define _FLUID_RVOICE_DSP_H

#include <stdbool.h>

#include "fluid_rvoice.h"

#define CONCAT(a, b, c, d) a##b##c##d

#define FLUID_RVOICE_GET_FLOAT_SAMPLE(is_24bit) CONCAT(fluid_rvoice_get_float_sample_, is_24bit, , )

#define FLUID_RVOICE_DSP_SILENCE_LOCAL(looping) CONCAT(fluid_rvoice_dsp_silence_local_, looping, , )

#define FLUID_RVOICE_DSP_INTERPOLATE_NONE_LOCAL(is_24bit, looping) CONCAT(fluid_rvoice_dsp_interpolate_none_local_, is_24bit, _, looping)
#define FLUID_RVOICE_DSP_INTERPOLATE_LINEAR_LOCAL(is_24bit, looping) CONCAT(fluid_rvoice_dsp_interpolate_linear_local_, is_24bit, _, looping)
#define FLUID_RVOICE_DSP_INTERPOLATE_4TH_ORDER_LOCAL(is_24bit, looping) CONCAT(fluid_rvoice_dsp_interpolate_4th_order_local_, is_24bit, _, looping)
#define FLUID_RVOICE_DSP_INTERPOLATE_7TH_ORDER_LOCAL(is_24bit, looping) CONCAT(fluid_rvoice_dsp_interpolate_7th_order_local_, is_24bit, _, looping)

#define FLUID_RVOICE_DSP_INTERPOLATE_LOCAL(is_24bit, looping) CONCAT(fluid_rvoice_dsp_interpolate_local_, is_24bit, _, looping)

static FLUID_INLINE fluid_real_t
FLUID_RVOICE_GET_FLOAT_SAMPLE(true)(const short int *FLUID_RESTRICT dsp_msb, const char *FLUID_RESTRICT dsp_lsb, unsigned int idx)
{
    int32_t sample = fluid_rvoice_get_sample24(dsp_msb, dsp_lsb, idx);
    return (fluid_real_t)sample;
}

static FLUID_INLINE fluid_real_t
FLUID_RVOICE_GET_FLOAT_SAMPLE(false)(const short int *FLUID_RESTRICT dsp_msb, const char *FLUID_RESTRICT dsp_lsb, unsigned int idx)
{
    int32_t sample = fluid_rvoice_get_sample16(dsp_msb, idx);
    return (fluid_real_t)sample;
}

#endif
