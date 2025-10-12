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

#include "fluid_rvoice_dsp.h"
#include "fluid_sys.h"
#include "fluid_rvoice.h"

#define LOOPING false

#include "fluid_rvoice_dsp_silence.inl"

#define IS_24BIT false
#include "fluid_rvoice_dsp_interpolate.inl"
#undef IS_24BIT
#define IS_24BIT true
#include "fluid_rvoice_dsp_interpolate.inl"
#undef IS_24BIT

#undef LOOPING
#define LOOPING true

#include "fluid_rvoice_dsp_silence.inl"

#define IS_24BIT false
#include "fluid_rvoice_dsp_interpolate.inl"
#undef IS_24BIT
#define IS_24BIT true
#include "fluid_rvoice_dsp_interpolate.inl"
#undef IS_24BIT

#undef LOOPING

extern "C" int
fluid_rvoice_dsp_silence(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf, int looping)
{
    if (looping)
    {
        return FLUID_RVOICE_DSP_SILENCE_LOCAL(true)(rvoice, dsp_buf);
    }
    else
    {
        return FLUID_RVOICE_DSP_SILENCE_LOCAL(false)(rvoice, dsp_buf);
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
            return FLUID_RVOICE_DSP_INTERPOLATE_LOCAL(true, true)(rvoice, dsp_buf);
        }
        else
        {
            return FLUID_RVOICE_DSP_INTERPOLATE_LOCAL(true, false)(rvoice, dsp_buf);
        }
    }
    else
    {
        // This case is most common, thanks to templating it will also become the fastest one
        if (looping)
        {
            return FLUID_RVOICE_DSP_INTERPOLATE_LOCAL(false, true)(rvoice, dsp_buf);
        }
        else
        {
            return FLUID_RVOICE_DSP_INTERPOLATE_LOCAL(false, false)(rvoice, dsp_buf);
        }
    }
}