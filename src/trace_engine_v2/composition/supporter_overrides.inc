#pragma once

// Centralize the late Supporter wrapper families behind one composition boundary.
// Their order remains behavior-significant because later wrappers observe the
// implementation exposed by earlier includes.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// Gladion wrapper stack. Keeping this chain at the root Supporter boundary removes
// an include-only composition layer while preserving the exact macro sequence.
#ifdef play_gladion
#error "Gladion composition requires play_gladion to be undefined on entry"
#endif

#define play_gladion play_gladion_prize_payload_original
#include "../part_012_gladion_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1030_original
#include "../part_prize_payload_outlet_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1191_original
#include "../part_issue_1030_gladion_turo_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1595_original
#include "../part_issue_1191_gladion_steven_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1608_original
#include "../part_issue_1595_gladion_grass_turo_blender_override.inc"
#undef play_gladion

#include "../part_issue_1608_burnet_before_dead_crispin_override.inc"

#ifdef play_gladion
#error "Gladion composition leaked play_gladion"
#endif

// Arven wrapper stack. The root Supporter boundary now owns this short chain too,
// avoiding another one-purpose include file while retaining its alias guards.
#ifdef play_arven
#error "Arven composition requires play_arven to be undefined on entry"
#endif

#define play_arven play_arven_powerglass_original
#include "../part_012_override.inc"
#undef play_arven

#define play_arven play_arven_fss_blender_contention_original
#include "../part_012_powerglass_override.inc"
#undef play_arven

#define play_arven play_arven_empty_deck_original
#include "../part_012_arven_fss_blender_contention_override.inc"
#undef play_arven

#ifdef play_arven
#error "Arven composition leaked play_arven"
#endif

#include "supporter_selector_overrides.inc"
