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
#include "fluid_rvoice.h"

template<bool IS_24BIT>
static FLUID_INLINE fluid_real_t
fluid_rvoice_get_float_sample(const short int *FLUID_RESTRICT dsp_msb, const char *FLUID_RESTRICT dsp_lsb, unsigned int idx)
{
    int32_t sample;
    if (IS_24BIT)
    {
        sample = fluid_rvoice_get_sample24(dsp_msb, dsp_lsb, idx);
    }
    else
    {
        sample = fluid_rvoice_get_sample16(dsp_msb, idx);
    }
    
    return (fluid_real_t)sample;
}

#include "fluid_rvoice_dsp_silence.inl"
#include "fluid_rvoice_dsp_interpolate.inl"

extern "C" int
fluid_rvoice_dsp_silence(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf, int looping)
{
    if (looping)
    {
        return fluid_rvoice_dsp_silence_local<true>(rvoice, dsp_buf);
    }
    else
    {
        return fluid_rvoice_dsp_silence_local<false>(rvoice, dsp_buf);
    }
}

extern "C" int
fluid_rvoice_dsp_interpolate(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf, int looping)
{
    bool is_24bit = rvoice->dsp.sample->data24 != NULL;

    if (is_24bit)
    {
        if (looping)
        {
            return fluid_rvoice_dsp_interpolate_local<true, true>(rvoice, dsp_buf);
        }
        else
        {
            return fluid_rvoice_dsp_interpolate_local<true, false>(rvoice, dsp_buf);
        }
    }
    else
    {
        // This case is most common, thanks to templating it will also become the fastest one
        if (looping)
        {
            return fluid_rvoice_dsp_interpolate_local<false, true>(rvoice, dsp_buf);
        }
        else
        {
            return fluid_rvoice_dsp_interpolate_local<false, false>(rvoice, dsp_buf);
        }
    }
}