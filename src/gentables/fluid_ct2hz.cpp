
#include "utils/fluid_conv_tables.h"
#include "fluidsynth_priv.h"

#include "gcem.hpp"

extern "C" const fluid_real_t fluid_ct2hz_tab[] =
{
// 6,875 is just a factor that we already multiply into the lookup table to save
// that multiplication in fluid_ct2hz_real()
// 6.875 Hz because 440Hz / 2^6
#define X(i) (fluid_real_t)(6.875 * gcem::pow(2.0, static_cast<fluid_real_t>(i) / 1200.0)),
#define AUTO_GEN_ARRAY_SIZE FLUID_CENTS_HZ_SIZE
#include "auto_gen_array.h"
};
