#pragma once

// Compose the legacy supporter body in one place while preserving the Forest Seal
// Stone alias for the later burnet_fss_overrides.inc wrapper chain.
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

// Intentionally leave use_fss defined. composition/burnet_fss_overrides.inc consumes
// and replaces this alias after part_014a.inc reaches the required member boundary.
// Wrapper continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/burnet_fss_overrides.inc
