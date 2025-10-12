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
#include "fluid_phase.h"
#include "fluid_rvoice.h"

/* Special case of interpolate_none for rendering silent voices, i.e. in delay phase or zero volume */
static int
FLUID_RVOICE_DSP_SILENCE_LOCAL(LOOPING)(fluid_rvoice_t *rvoice, fluid_real_t *FLUID_RESTRICT dsp_buf)
{
    fluid_rvoice_dsp_t *voice = &rvoice->dsp;
    fluid_phase_t dsp_phase = voice->phase;
    fluid_phase_t dsp_phase_incr;
    unsigned short dsp_i = 0;
    unsigned int dsp_phase_index;
    unsigned int end_index;

    /* Convert playback "speed" floating point value to phase index/fract */
    fluid_phase_set_float(dsp_phase_incr, voice->phase_incr);

    end_index = LOOPING ? voice->loopend - 1 : voice->end;

    while (1)
    {
        dsp_phase_index = fluid_phase_index_round(dsp_phase); /* round to nearest point */

        /* interpolate sequence of sample points */
        for (; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
        {
            fluid_real_t sample = 0;
            dsp_buf[dsp_i] = sample;

            /* increment phase and amplitude */
            fluid_phase_incr(dsp_phase, dsp_phase_incr);
        }

        /* break out if not looping (buffer may not be full) */
        if (!LOOPING)
        {
            break;
        }

        dsp_phase_index = fluid_phase_index_round(dsp_phase); /* round to nearest point */
        /* go back to loop start */
        if (dsp_phase_index > end_index)
        {
            fluid_phase_sub_int(dsp_phase, voice->loopend - voice->loopstart);
            voice->has_looped = 1;
        }

        /* break out if filled buffer */
        if (dsp_i >= FLUID_BUFSIZE)
        {
            break;
        }
    }

    voice->phase = dsp_phase;
    // Note, there is no need to update the amplitude here. When the voice becomes audible again, the amp will be updated anyway in fluid_rvoice_calc_amp().
    // voice->amp = dsp_amp;

    return (dsp_i);
}
