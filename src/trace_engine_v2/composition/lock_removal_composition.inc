#pragma once

// Canonical #3057 lock-removal composition boundary.
// Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
// Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
// Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
// Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057

// Stage 1: load the shared forest / Field Blower policy before exporting aliases.
#include "../core/forest_field_blower_policy.inc"

// Stage 2: expose only the two lock-removal wrappers required by post-search composition.
#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition requires lock-removal aliases to be undefined"
#endif
#define play_chaotic_swell play_chaotic_swell_issue_3057
#define play_field_blower play_field_blower_issue_3057
#include "post_014a_overrides.inc"
#undef play_field_blower
#undef play_chaotic_swell

// Stage 3: enforce a clean macro boundary for late execution/reporting.
#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition leaked a temporary lock-removal alias"
#endif
