/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "fluid_sys.h"
#include "fluid_iir_filter.h"
#include "fluid_conv.h"

#include <cmath>

template<typename R, bool GAIN_NORM, enum fluid_iir_filter_type TYPE>
static inline void fluid_iir_filter_calculate_coefficients(R fres,
                                                           R q,
                                                           fluid_iir_sincos_t *sincos_table,
                                                           R *FLUID_RESTRICT a1_out,
                                                           R *FLUID_RESTRICT a2_out,
                                                           R *FLUID_RESTRICT b02_out,
                                                           R *FLUID_RESTRICT b1_out)
{
    R filter_gain = 1.0f;

    /*
     * Those equations from Robert Bristow-Johnson's `Cookbook
     * formulae for audio EQ biquad filter coefficients', obtained
     * from Harmony-central.com / Computer / Programming. They are
     * the result of the bilinear transform on an analogue filter
     * prototype. To quote, `BLT frequency warping has been taken
     * into account for both significant frequency relocation and for
     * bandwidth readjustment'. */

    signed tab_idx = (static_cast<signed>(fres) - FRES_MIN) / CENTS_STEP;
#ifndef DBG_FILTER
    fluid_clip(tab_idx, 0, SINCOS_TAB_SIZE - 1);
#endif
    R sin_coeff = sincos_table[tab_idx].sin;
    R cos_coeff = sincos_table[tab_idx].cos;
    R alpha_coeff = sin_coeff / (2.0f * q);
    R a0_inv = 1.0f / (1.0f + alpha_coeff);

    /* Calculate the filter coefficients. All coefficients are
     * normalized by a0. Think of `a1' as `a1/a0'.
     *
     * Here a couple of multiplications are saved by reusing common expressions.
     * The original equations should be:
     *  iir_filter->b0=(1.-cos_coeff)*a0_inv*0.5*filter_gain;
     *  iir_filter->b1=(1.-cos_coeff)*a0_inv*filter_gain;
     *  iir_filter->b2=(1.-cos_coeff)*a0_inv*0.5*filter_gain; */

    /* "a" coeffs are same for all 3 available filter types */
    R a1_temp = -2.0f * cos_coeff * a0_inv;
    R a2_temp = (1.0f - alpha_coeff) * a0_inv;
    R b02_temp, b1_temp;

    if (GAIN_NORM)
    {
        /* SF 2.01 page 59:
         *
         *  The SoundFont specs ask for a gain reduction equal to half the
         *  height of the resonance peak (Q).  For example, for a 10 dB
         *  resonance peak, the gain is reduced by 5 dB.  This is done by
         *  multiplying the total gain with sqrt(1/Q).  `Sqrt' divides dB
         *  by 2 (100 lin = 40 dB, 10 lin = 20 dB, 3.16 lin = 10 dB etc)
         *  The gain is later factored into the 'b' coefficients
         *  (numerator of the filter equation).  This gain factor depends
         *  only on Q, so this is the right place to calculate it.
         */
        filter_gain /= std::sqrt(q);
    }

    switch (TYPE)
    {
        case FLUID_IIR_HIGHPASS:
            b1_temp = (1.0f + cos_coeff) * a0_inv * filter_gain;
    
            /* both b0 -and- b2 */
            b02_temp = b1_temp * 0.5f;
    
            b1_temp *= -1.0f;
            break;
    
        case FLUID_IIR_LOWPASS:
            b1_temp = (1.0f - cos_coeff) * a0_inv * filter_gain;
    
            /* both b0 -and- b2 */
            b02_temp = b1_temp * 0.5f;
            break;
    
        default:
            /* filter disabled, should never get here */
            return;
    }

    *a1_out = a1_temp;
    *a2_out = a2_temp;
    *b02_out = b02_temp;
    *b1_out = b1_temp;
}
