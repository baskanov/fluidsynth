#ifndef _FLUID_IIR_FILTER_IMPL_H
#define _FLUID_IIR_FILTER_IMPL_H

#define CONCAT(a, b, c, d, e, f) a##b##c##d##e##f

#define FLUID_IIR_FILTER_CALCULATE_COEFFICIENTS(r, gain_norm, type) CONCAT(fluid_iir_filter_calculate_coefficients_, r, _, gain_norm, _, type)
#define FLUID_IIR_FILTER_APPLY_LOCAL(gain_norm, amplify, type) CONCAT(fluid_iir_filter_apply_local_, gain_norm, _, amplify, _, type)

#endif
