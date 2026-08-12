#pragma once

// Compose the #3057 Pineco/Secret Box lock-removal wrapper at one named boundary.
// Keep alias setup and teardown together so callers cannot accidentally expose
// the replacement members beyond the intended textual-composition window.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Forest / Field Blower / Secret Box: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sv6-163
// Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
#include "../part_issue_3057_swell_field_blower_route.inc"
#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition requires lock-removal aliases to be undefined"
#endif
#define play_chaotic_swell play_chaotic_swell_issue_3057
#define play_field_blower play_field_blower_issue_3057
#include "post_014a_overrides.inc"
#undef play_field_blower
#undef play_chaotic_swell
#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition leaked a temporary lock-removal alias"
#endif
