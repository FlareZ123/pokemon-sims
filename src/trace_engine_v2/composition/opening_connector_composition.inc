#pragma once

// Canonical opening connector composition. Keep post-evolution discard provenance,
// Tate handling, and the following Item search wrapper family in historical order.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Opening composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
#include "../part_discard_recovery_provenance_override.inc"
#include "../part_tate_blender_attachment_override.inc"
#include "../part_tate_blender_tate_override.inc"

#if defined(play_mysterious_treasure) || defined(play_heavy_ball) || \
    defined(play_quick_ball) || defined(play_ultra_ball) || defined(in_play)
#error "Tate search-item composition requires wrapper aliases to be undefined on entry"
#endif

#define play_mysterious_treasure play_mysterious_treasure_empty_deck_original
#define play_heavy_ball play_heavy_ball_prize_payload_original
#include "../part_008b.inc"
#undef play_heavy_ball

// Wonder Tag is copy-specific. Preserve all existing selector controls while an
// in-play Tapu no longer suppresses a second physical copy:
// https://api.pokemontcg.io/v2/cards/sm2-60
// https://github.com/FlareZ123/pokemon-sims/issues/746
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009a.inc"
#undef in_play
#undef play_mysterious_treasure

// Keep the historical base compiled for regression archaeology, but expose the
// card-class-backed base under the legacy wrapper name consumed by later layers.
#define play_quick_ball play_quick_ball_card_class_legacy_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009b1.inc"
#undef in_play
#undef play_quick_ball

// part_009b2.inc now owns the one live Quick Ball card-class composition boundary.
// Keeping that include centralized avoids a second guarded include at this layer.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Quick Ball boundary repair: https://github.com/FlareZ123/pokemon-sims/issues/3341
#define play_ultra_ball play_ultra_ball_original
#include "../part_009b2.inc"

#if defined(play_mysterious_treasure) || defined(play_heavy_ball) || \
    defined(play_quick_ball) || defined(in_play)
#error "Tate search-item composition leaked a local wrapper alias"
#endif

// play_ultra_ball intentionally remains exported until the following Steven/Blender
// layer releases it at the same historical boundary.
