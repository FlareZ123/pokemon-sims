#ifndef POKEMON_SIMS_FINAL_ENGINE_OVERRIDES_INCLUDED
#define POKEMON_SIMS_FINAL_ENGINE_OVERRIDES_INCLUDED

// Keep the late Engine policy fragments in their established dependency order so
// regidrago_sim.cpp owns one composition boundary instead of four scattered includes.
#include "legacy_star_overrides.inc"
#include "supporter_overrides.inc"
#include "search_item_overrides.inc"
#include "secret_box_trace_overrides.inc"

#endif  // POKEMON_SIMS_FINAL_ENGINE_OVERRIDES_INCLUDED
