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
// Legacy Supporters: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/legacy_supporter_body.inc
// Supporter selector: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/supporter_selector_body.inc
// VSTAR powers: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/vstar_power_body.inc
#include "tate_discard_overrides.inc"
#include "steven_blender_overrides.inc"
#include "legacy_supporter_body.inc"
#include "supporter_selector_body.inc"
#include "vstar_power_body.inc"
