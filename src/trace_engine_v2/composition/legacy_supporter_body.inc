#pragma once

// Compose the legacy Supporter body, its immediately adjacent Supporter selector,
// and the following VSTAR-power body in one ordered unit. Keeping these macro
// lifetimes together makes the historical part_011 -> part_012 -> part_013 member
// boundary explicit without changing textual order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
#if defined(use_fss) || defined(play_crispin) || \
    defined(play_professor_burnet) || defined(play_steven)
#error "Legacy Supporter composition requires aliases to be undefined on entry"
#endif

#define use_fss use_fss_latias_original
#define play_crispin play_crispin_empty_deck_original
#define play_professor_burnet play_professor_burnet_legacy_original
#define play_steven play_steven_empty_deck_original
#include "../part_011.inc"
#undef play_steven
#undef play_professor_burnet
#undef play_crispin

#if defined(play_crispin) || defined(play_professor_burnet) || defined(play_steven)
#error "Legacy Supporter composition leaked a local alias"
#endif

// Intentionally leave use_fss defined. post_014a_overrides.inc consumes and
// replaces this alias after part_014a.inc reaches the required member boundary.
// Continuation owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

// The part_012 selector follows part_011 immediately in the historical composition.
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
#if defined(play_arven) || defined(play_gladion) || defined(play_serena)
#error "Supporter selector composition requires aliases to be undefined on entry"
#endif

#define play_arven play_arven_original
#define play_gladion play_gladion_original
#define play_serena play_serena_issue1683_original
#include "../part_012.inc"
#undef play_serena
#undef play_gladion
#undef play_arven

#if defined(play_arven) || defined(play_gladion) || defined(play_serena)
#error "Supporter selector composition leaked a local alias"
#endif

// The part_013 VSTAR-power aliases follow the selector immediately. Preserve the
// Celestial Roar alias for the later wrapper chain.
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
#if defined(use_celestial_roar) || defined(use_legacy_star)
#error "VSTAR-power composition requires aliases to be undefined on entry"
#endif

#define use_celestial_roar use_celestial_roar_original
#define use_legacy_star use_legacy_star_original
#include "../part_013.inc"
#undef use_legacy_star

#if defined(use_legacy_star)
#error "VSTAR-power composition leaked use_legacy_star"
#endif

// Intentionally leave use_celestial_roar defined. post_014a_overrides.inc consumes
// and replaces this alias after the part_014a boundary.
// Continuation owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
