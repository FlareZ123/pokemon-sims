#pragma once

// Compose the member overrides that become legal immediately after part_014a.inc
// closes run_search_items_one_step(). Keeping this boundary named avoids exposing
// independent override families in the translation-unit root while preserving
// their established dependency order.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135

#include "../part_issue_1683_serena_dead_quick_ball_override.inc"

// Keep the post-part_014a Burnet and Forest Seal Stone aliases in the same
// composition spine instead of routing them through one-purpose wrapper files.
#define play_professor_burnet play_professor_burnet_empty_deck_original
#include "../part_011_burnet_thinning_override.inc"
#undef play_professor_burnet

#undef use_fss
#define use_fss use_fss_empty_deck_original
#include "../part_011_fss_latias_override.inc"
#undef use_fss

// Preserve the Celestial Roar alias bridge immediately before the final override
// families, matching the prior wrapper's exact preprocessing lifetime and order.
#undef use_celestial_roar
#define use_celestial_roar use_celestial_roar_issue1369_original
#include "../part_celestial_roar_override.inc"
#undef use_celestial_roar

// Keep the final policy fragments directly on this named post-search spine so the
// engine no longer crosses another wrapper whose only job is forwarding includes.
#include "legacy_star_projection_chain.inc" // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
#include "gladion_overrides.inc" // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
#include "arven_overrides.inc" // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
#include "supporter_selector_overrides.inc" // Supporter rule: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
#include "search_item_overrides.inc"
#include "secret_box_trace_overrides.inc"
