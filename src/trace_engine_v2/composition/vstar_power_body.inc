#pragma once

// Compose the part_013 VSTAR-power aliases at the same textual boundary while
// preserving Celestial Roar for its later wrapper chain.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
#if defined(use_celestial_roar) || defined(use_legacy_star)
#error "VSTAR-power composition requires aliases to be undefined on entry"
#endif

#define use_celestial_roar use_celestial_roar_original
#define use_legacy_star use_legacy_star_original
#include "../part_013.inc"
#undef use_legacy_star

#ifdef use_legacy_star
#error "VSTAR-power composition leaked use_legacy_star"
#endif
#ifndef use_celestial_roar
#error "VSTAR-power composition must preserve the Celestial Roar alias"
#endif

// Intentionally leave use_celestial_roar defined. The later Celestial Roar
// composition consumes and replaces this alias after the part_014a boundary.
// Wrapper continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/celestial_roar_overrides.inc
