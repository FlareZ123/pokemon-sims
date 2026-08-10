#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#if defined(bench_from_hand) || defined(bench_oricorio_if_useful) || \
    defined(needs_oricorio_connector) || defined(needs_tapu_connector) || \
    defined(bench_tapu_if_useful) || defined(recover_discard_to_hand) || \
    defined(choose_discard)
#error "Opening Engine composition requires its local aliases to be undefined on entry"
#endif

#include "../part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "../part_begin_turn_override.inc"
#include "../part_tapu_copy_availability_override.inc"
#define bench_from_hand bench_from_hand_empty_deck_original
#define bench_oricorio_if_useful bench_oricorio_if_useful_empty_deck_original
#define needs_oricorio_connector needs_oricorio_connector_original
#define needs_tapu_connector needs_tapu_connector_original
#define bench_tapu_if_useful bench_tapu_if_useful_original
#define recover_discard_to_hand recover_discard_to_hand_name_only_original
#define choose_discard choose_discard_issue1436_original
#include "../part_006.inc"
#undef recover_discard_to_hand
#undef bench_tapu_if_useful
#undef needs_tapu_connector
#undef needs_oricorio_connector
#undef bench_oricorio_if_useful
#undef bench_from_hand

// choose_discard intentionally remains exported through part_007.inc and
// part_008a.inc; src/regidrago_sim.cpp releases it at that legacy boundary.
#if defined(bench_from_hand) || defined(bench_oricorio_if_useful) || \
    defined(needs_oricorio_connector) || defined(needs_tapu_connector) || \
    defined(bench_tapu_if_useful) || defined(recover_discard_to_hand)
#error "Opening Engine composition leaked a local alias"
#endif
