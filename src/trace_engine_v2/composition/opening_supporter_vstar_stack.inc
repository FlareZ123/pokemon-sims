#pragma once

// Centralized Supporter/VSTAR tail. use_fss and use_celestial_roar remain
// intentionally exported to post_014a_overrides.inc.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

// Preserve the Forest Seal Stone alias for the later post-part_014a wrapper chain.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
#define use_fss use_fss_latias_original
#define play_crispin play_crispin_empty_deck_original
#define play_professor_burnet play_professor_burnet_legacy_original
#define play_steven play_steven_empty_deck_original
#include "../part_011.inc"
#undef play_steven
#undef play_professor_burnet
#undef play_crispin
// use_fss intentionally remains defined for the post-part_014a FSS composition.
// Continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

// Keep the part_012 Supporter selector aliases at this composition boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
#define play_arven play_arven_original
#define play_gladion play_gladion_original
#define play_serena play_serena_issue1683_original
#include "../part_012.inc"
#undef play_serena
#undef play_gladion
#undef play_arven

// Preserve Celestial Roar for its later post-part_014a wrapper chain while
// keeping the part_013 VSTAR-power aliases in this parent composition stage.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
#define use_celestial_roar use_celestial_roar_original
#define use_legacy_star use_legacy_star_original
#include "../part_013.inc"
#undef use_legacy_star
// use_celestial_roar intentionally remains defined for the later composition.
// Continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
