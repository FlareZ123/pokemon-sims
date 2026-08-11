#pragma once

// Compose the legacy Supporter and Supporter-selector bodies in one ordered unit
// while preserving the Forest Seal Stone alias for the later
// burnet_fss_overrides.inc wrapper chain.
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

// Intentionally leave use_fss defined. composition/burnet_fss_overrides.inc consumes
// and replaces this alias after part_014a.inc reaches the required member boundary.
// Wrapper continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/burnet_fss_overrides.inc

// Keep the immediately following part_012 Supporter selector aliases beside the
// part_011 Supporter body so their historical textual order is visible in one place.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
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
