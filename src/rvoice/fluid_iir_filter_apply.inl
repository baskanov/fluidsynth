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

/**
 * Applies a low- or high-pass filter with variable cutoff frequency and quality factor
 * for a given biquad transfer function:
 *          b0 + b1*z^-1 + b2*z^-2
 *  H(z) = ------------------------
 *          a0 + a1*z^-1 + a2*z^-2
 *
 * Also modifies filter state accordingly.
 * @param iir_filter Filter parameter
 * @param dsp_buf Pointer to the synthesized audio data
 * @param count Count of samples in dsp_buf
 */
/*
 * Variable description:
 * - dsp_a1, dsp_a2: Filter coefficients for the the previously filtered output signal
 * - dsp_b0, dsp_b1, dsp_b2: Filter coefficients for input signal
 * - coefficients normalized to a0
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_i: Index through the output buffer
 * - dsp_centernode: delay line for the IIR filter
 * - dsp_hist1: same
 * - dsp_hist2: same
 */
static void
FLUID_IIR_FILTER_APPLY_LOCAL(GAIN_NORM, AMPLIFY, TYPE)(fluid_iir_filter_t *iir_filter, fluid_real_t *dsp_buf, unsigned int count)
{
    // FLUID_IIR_Q_LINEAR may switch the filter off by setting Q==0
    // Due to the linear smoothing, last_q may not exactly become zero.
    if (iir_filter->type == FLUID_IIR_DISABLED || iir_filter->last_q < Q_MIN)
    {
        return;
    }
    else
    {
        /* IIR filter sample history */
        fluid_real_t dsp_hist1 = iir_filter->hist1;
        fluid_real_t dsp_hist2 = iir_filter->hist2;

        /* IIR filter coefficients */
        IIR_COEFF_T dsp_a1 = iir_filter->a1;
        IIR_COEFF_T dsp_a2 = iir_filter->a2;
        IIR_COEFF_T dsp_b02 = iir_filter->b02;
        IIR_COEFF_T dsp_b1 = iir_filter->b1;

        int fres_incr_count = iir_filter->fres_incr_count;
        int q_incr_count = iir_filter->q_incr_count;
        
        fluid_real_t dsp_amp = iir_filter->amp;
        fluid_real_t dsp_amp_incr = iir_filter->amp_incr;
        IIR_COEFF_T fres = static_cast<IIR_COEFF_T>(iir_filter->last_fres);
        IIR_COEFF_T q = static_cast<IIR_COEFF_T>(iir_filter->last_q);
        
        const IIR_COEFF_T fres_incr = static_cast<IIR_COEFF_T>(iir_filter->fres_incr);
        const IIR_COEFF_T q_incr = static_cast<IIR_COEFF_T>(iir_filter->q_incr);

        /* filter (implement the voice filter according to SoundFont standard) */

        unsigned int dsp_i;
        for (dsp_i = 0; dsp_i < count; dsp_i++)
        {
            /* The filter is implemented in Direct-II form. */
            fluid_real_t dsp_centernode = dsp_buf[dsp_i] - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
            fluid_real_t sample = dsp_b02 * (dsp_centernode + dsp_hist2) + dsp_b1 * dsp_hist1;
            dsp_hist2 = dsp_hist1;
            dsp_hist1 = dsp_centernode;

            FLUID_ASSERT(dsp_hist1 == dsp_hist1);
            FLUID_ASSERT(sample == sample);
            FLUID_ASSERT(dsp_a1 == dsp_a1);
            FLUID_ASSERT(dsp_a2 == dsp_a2);
            FLUID_ASSERT(dsp_b02 == dsp_b02);
            FLUID_ASSERT(dsp_b1 == dsp_b1);
            FLUID_ASSERT(q >= Q_MIN);

            /* Alternatively, it could be implemented in Transposed Direct Form II */
            // fluid_real_t dsp_input = dsp_buf[dsp_i];
            // dsp_buf[dsp_i] = dsp_b02 * dsp_input + dsp_hist1;
            // dsp_hist1 = dsp_b1 * dsp_input - dsp_a1 * dsp_buf[dsp_i] + dsp_hist2;
            // dsp_hist2 = dsp_b02 * dsp_input - dsp_a2 * dsp_buf[dsp_i];

            if(AMPLIFY)
            {
                dsp_buf[dsp_i] = dsp_amp * sample;
                dsp_amp += dsp_amp_incr;
            }
            else
            {
                dsp_buf[dsp_i] = sample;
            }

            if(fres_incr_count > 0 || q_incr_count > 0)
            {
                if(fres_incr_count > 0)
                {
                    --fres_incr_count;
                    fres += fres_incr;
                }
                if(q_incr_count > 0)
                {
                    --q_incr_count;
                    q += q_incr;
                    if(q < Q_MIN)
                    {
                        LOG_FILTER("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                        LOG_FILTER("!!!OOPS!!! limited Q to its minimum value, was: %f", q);
                        LOG_FILTER("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                        q_incr_count = 0;
                        q = Q_MIN;
                    }
                }

                LOG_FILTER("fres: %.2f Hz  | target_fres: %.2f Hz | fres_incr: %f\t| fres_incr_count: %d\t|---| q: %f\t| target_q: %f\t| q_incr: %f\t| q_incr_count: %d", fres, iir_filter->target_fres, fres_incr, fres_incr_count, q, iir_filter->target_q, q_incr, q_incr_count);
                
                FLUID_IIR_FILTER_CALCULATE_COEFFICIENTS(IIR_COEFF_T, GAIN_NORM, TYPE)(fres, q, iir_filter->sincos_table, &dsp_a1, &dsp_a2, &dsp_b02, &dsp_b1);
            }
        }

        iir_filter->a1 = dsp_a1;
        iir_filter->a2 = dsp_a2;
        iir_filter->b02= dsp_b02;
        iir_filter->b1 = dsp_b1;

        /* Check for denormal number (too close to zero). */
        if (FLUID_FABS(dsp_hist1) < 1e-20f)
        {
            dsp_hist1 = 0.0f;
        }
        if (FLUID_FABS(dsp_hist2) < 1e-20f)
        {
            dsp_hist2 = 0.0f;
        }
        iir_filter->hist1 = dsp_hist1;
        iir_filter->hist2 = dsp_hist2;

        iir_filter->last_fres = fres;
        iir_filter->fres_incr_count = fres_incr_count;
        iir_filter->last_q = q;
        iir_filter->q_incr_count = q_incr_count;
        iir_filter->amp = dsp_amp;
    }
}
