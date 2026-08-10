#pragma once

// Isolate the Celestial Roar alias bridge from the root translation unit while
// preserving the exact include order required by the legacy split source.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135

#undef use_celestial_roar
#define use_celestial_roar use_celestial_roar_issue1369_original
#include "../part_celestial_roar_override.inc"
#undef use_celestial_roar
