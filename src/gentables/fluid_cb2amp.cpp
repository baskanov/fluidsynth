
#include "utils/fluid_conv_tables.h"
#include "fluidsynth_priv.h"

#define GCEM_E static_cast<double>(2.7182818284590452353602874713526624977572L)
#include "gcem.hpp"

extern "C" const fluid_real_t fluid_cb2amp_tab_cpp[] =
{
/* centibels to amplitude conversion
 * Note: SF2.01 section 8.1.3: Initial attenuation range is
 * between 0 and 144 dB. Therefore a negative attenuation is
 * not allowed.
 */
#define X(i) (fluid_real_t)gcem::pow(10.0, static_cast<fluid_real_t>(i) / -200.0),
#define AUTO_GEN_ARRAY_SIZE FLUID_CB_AMP_SIZE
#include "auto_gen_array.h"
};

extern "C" const fluid_real_t *const fluid_cb2amp_tab = fluid_cb2amp_tab_cpp;
