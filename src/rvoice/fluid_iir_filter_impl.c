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

#include "fluid_iir_filter_impl.h"
#include "fluid_sys.h"
#include "fluid_iir_filter.h"
#include "fluid_conv.h"

#include <stdbool.h>


// Calculating the sine and cosine coefficients for every possible cutoff frequency is too CPU expensive and can harm real-time playback.
// Therefore, we precalculate the coefficients with a precision of CENTS_STEP and store them in a table.
void fluid_iir_filter_init_table(fluid_iir_sincos_t *sincos_table, fluid_real_t sample_rate)
{
    const IIR_COEFF_T period = (IIR_COEFF_T)(2.0 * M_PI / sample_rate);
    int fres_cents, i;
    for(fres_cents = FRES_MIN, i=0; fres_cents <= FRES_MAX; fres_cents += CENTS_STEP, i++)
    {
        IIR_COEFF_T fres = (IIR_COEFF_T)fluid_ct2hz((fluid_real_t)fres_cents);
        IIR_COEFF_T omega = period * fres;
        IIR_COEFF_T sin_coeff = FLUID_SIN(omega);
        IIR_COEFF_T cos_coeff = FLUID_COS(omega);
        // i == (fres_cents - FRES_MIN) / CENTS_STEP;
        sincos_table[i].sin = sin_coeff;
        sincos_table[i].cos = cos_coeff;
    }
}

#define R IIR_COEFF_T

#define GAIN_NORM false

#define TYPE FLUID_IIR_HIGHPASS
#include "fluid_iir_filter_calculate_coefficients.inl"
#define AMPLIFY false
#include "fluid_iir_filter_apply.inl"
#undef AMPLIFY

#undef TYPE
#define TYPE FLUID_IIR_LOWPASS

#include "fluid_iir_filter_calculate_coefficients.inl"
#define AMPLIFY false
#include "fluid_iir_filter_apply.inl"
#undef AMPLIFY

#undef TYPE

#undef GAIN_NORM
#define GAIN_NORM true

#define TYPE FLUID_IIR_HIGHPASS

#include "fluid_iir_filter_calculate_coefficients.inl"
#define AMPLIFY false
#include "fluid_iir_filter_apply.inl"
#undef AMPLIFY

#undef TYPE
#define TYPE FLUID_IIR_LOWPASS

#include "fluid_iir_filter_calculate_coefficients.inl"
#define AMPLIFY false
#include "fluid_iir_filter_apply.inl"
#undef AMPLIFY
#define AMPLIFY true
#include "fluid_iir_filter_apply.inl"
#undef AMPLIFY

#undef TYPE

#undef GAIN_NORM

#undef R

void fluid_iir_filter_apply(fluid_iir_filter_t *resonant_filter,
                                       fluid_iir_filter_t *resonant_custom_filter,
                                       fluid_real_t *dsp_buf,
                                       unsigned int count)
{
    if(resonant_custom_filter->flags & FLUID_IIR_NO_GAIN_AMP)
    {
        if(resonant_custom_filter->type == FLUID_IIR_HIGHPASS)
        {
            FLUID_IIR_FILTER_APPLY_LOCAL(false, false, FLUID_IIR_HIGHPASS)(resonant_custom_filter, dsp_buf, count);
        }
        else
        {
            FLUID_IIR_FILTER_APPLY_LOCAL(false, false, FLUID_IIR_LOWPASS)(resonant_custom_filter, dsp_buf, count);
        }
    }
    else
    {
        if(resonant_custom_filter->type == FLUID_IIR_HIGHPASS)
        {
            FLUID_IIR_FILTER_APPLY_LOCAL(true, false, FLUID_IIR_HIGHPASS)(resonant_custom_filter, dsp_buf, count);
        }
        else
        {
            FLUID_IIR_FILTER_APPLY_LOCAL(true, false, FLUID_IIR_LOWPASS)(resonant_custom_filter, dsp_buf, count);
        }
    }

    // This is the last filter in the chain - the default SF2 filter that always runs. This one must apply the final envelope gain.
    FLUID_IIR_FILTER_APPLY_LOCAL(true, true, FLUID_IIR_LOWPASS)(resonant_filter, dsp_buf, count);
}

void fluid_iir_filter_calc(fluid_iir_filter_t *iir_filter,
                           fluid_real_t output_rate,
                           fluid_real_t fres_mod)
{
    bool calc_coeff_flag = false;
    fluid_real_t fres, fres_diff;
    IIR_COEFF_T last_fres_f;
    IIR_COEFF_T last_q_f;
    
    if(iir_filter->type == FLUID_IIR_DISABLED)
    {
        return;
    }

    /* calculate the frequency of the resonant filter in Hz */
    fres = fluid_ct2hz(iir_filter->fres + fres_mod);

    /* I removed the optimization of turning the filter off when the
     * resonance frequency is above the maximum frequency. Instead, the
     * filter frequency is set to a maximum of 0.45 times the sampling
     * rate. For a 44100 kHz sampling rate, this amounts to 19845
     * Hz. The reason is that there were problems with anti-aliasing when the
     * synthesizer was run at lower sampling rates. Thanks to Stephan
     * Tassart for pointing me to this bug. By turning the filter on and
     * clipping the maximum filter frequency at 0.45*srate, the filter
     * is used as an anti-aliasing filter. */

    if(fres > 0.45f * output_rate)
    {
        fres = 0.45f * output_rate;
    }
    else if(fres < 5.f)
    {
        fres = 5.f;
    }

    LOG_FILTER("%f + %f = %f cents = %f Hz | Q: %f", iir_filter->fres, fres_mod, iir_filter->fres + fres_mod, fres, iir_filter->last_q);
    
    fres = fluid_hz2ct(fres);
    
    /* if filter enabled and there is a significant frequency change.. */
    fres_diff = fres - iir_filter->last_fres;
    if(iir_filter->filter_startup)
    {
        // The filter was just starting up, make sure to calculate initial coefficients for the initial Q value, even though the fres may not have changed
        calc_coeff_flag = true;
        
        iir_filter->fres_incr_count = 0;
        iir_filter->last_fres = fres;
        iir_filter->filter_startup = (iir_filter->last_q < Q_MIN); // filter coefficients will not be initialized when Q is small
    }
    else if(FLUID_FABS(fres_diff) > (fluid_real_t)CENTS_STEP) // only smooth out fres when difference is "significant"
    {
        fluid_real_t fres_incr_count = FLUID_BUFSIZE;
        fluid_real_t num_buffers = iir_filter->last_q;
        fluid_clip(num_buffers, 1, 5);
        // For high values of Q, the phase gets really steep. To prevent clicks when quickly modulating fres in this case, we need to smooth out "slower".
        // This is done by simply using Q times FLUID_BUFSIZE samples for the interpolation to complete, capped at 5.
        // 5 was chosen because the phase doesn't really get any steeper when continuing to increase Q.
        fres_incr_count *= num_buffers;
        iir_filter->fres_incr = fres_diff / (fres_incr_count);
        iir_filter->fres_incr_count = (int)(fres_incr_count + 0.5);

#ifdef DBG_FILTER
        iir_filter->target_fres = fres;
#endif

        // The filter coefficients have to be recalculated (filter cutoff has changed).
        calc_coeff_flag = true;
    }
    else
    {
        // difference in fres is small, so set it directly
        iir_filter->fres_incr_count = 0;
        iir_filter->last_fres = fres;
        // We do not account for any change of Q here - if it was changed q_incro_count will be non-zero and recalculating the coeffs
        // will be taken care of in fluid_iir_filter_apply().
    }

    last_fres_f = (IIR_COEFF_T)iir_filter->last_fres;
    last_q_f = (IIR_COEFF_T)iir_filter->last_q;
    if (calc_coeff_flag && !iir_filter->filter_startup)
    {
        if((iir_filter->flags & FLUID_IIR_NO_GAIN_AMP))
        {
            if(iir_filter->type == FLUID_IIR_HIGHPASS)
            {
                FLUID_IIR_FILTER_CALCULATE_COEFFICIENTS(IIR_COEFF_T, false, FLUID_IIR_HIGHPASS)(
                last_fres_f,
                last_q_f,
                iir_filter->sincos_table,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
            else
            {
                FLUID_IIR_FILTER_CALCULATE_COEFFICIENTS(IIR_COEFF_T, false, FLUID_IIR_LOWPASS)(
                last_fres_f,
                last_q_f,
                iir_filter->sincos_table,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
        }
        else
        {
            if(iir_filter->type == FLUID_IIR_HIGHPASS)
            {
                FLUID_IIR_FILTER_CALCULATE_COEFFICIENTS(IIR_COEFF_T, true, FLUID_IIR_HIGHPASS)(
                last_fres_f,
                last_q_f,
                iir_filter->sincos_table,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
            else
            {
                FLUID_IIR_FILTER_CALCULATE_COEFFICIENTS(IIR_COEFF_T, true, FLUID_IIR_LOWPASS)(
                last_fres_f,
                last_q_f,
                iir_filter->sincos_table,
                &iir_filter->a1,
                &iir_filter->a2,
                &iir_filter->b02,
                &iir_filter->b1);
            }
        }
    }
}
