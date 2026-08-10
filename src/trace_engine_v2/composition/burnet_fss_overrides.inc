#pragma once

// Keep the post-part_014a Burnet and Forest Seal Stone override aliases at one
// composition boundary while preserving the inherited use_fss alias ordering.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156

#define play_professor_burnet play_professor_burnet_empty_deck_original
#include "../part_011_burnet_thinning_override.inc"
#undef play_professor_burnet

#undef use_fss
#define use_fss use_fss_empty_deck_original
#include "../part_011_fss_latias_override.inc"
#undef use_fss
