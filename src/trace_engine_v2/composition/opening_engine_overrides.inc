#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "../part_begin_turn_override.inc"
#include "../core/tapu_connector_policy.inc"
#include "../core/payload_hand_policy.inc"
#include "../core/tapu_legacy_aliases.inc"

// Keep the coupled part_006/007/008a alias lifetime in one named boundary before
// the later override stages take ownership.
// Composition source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_body.inc
#include "opening_legacy_body.inc"

// Continue through the Tate/discard, Steven/Blender, legacy Supporter, Supporter
// selector, and VSTAR stages in historical order. Each composed boundary owns its
// local aliases; use_fss and use_celestial_roar intentionally remain live for the
// post-part_014a continuation.
// Tate/discard: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/tate_discard_overrides.inc
// Steven/Blender: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc
// Legacy Supporter cards: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/sm7-145
// Supporter selector cards: https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/swsh12-164
// VSTAR powers: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/vstar_power_body.inc
#include "tate_discard_overrides.inc"
#include "steven_blender_overrides.inc"

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

#include "vstar_power_body.inc"
