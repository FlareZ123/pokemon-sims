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

// Keep the legacy part_006 Bench, connector, recovery, and discard aliases inside
// their owning opening composition layer. choose_discard intentionally remains
// exported through part_007.inc and part_008a.inc for the established wrapper chain.
// Historical split source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_006.inc
#if defined(bench_from_hand) || defined(bench_oricorio_if_useful) || \
    defined(needs_oricorio_connector) || defined(needs_tapu_connector) || \
    defined(bench_tapu_if_useful) || defined(recover_discard_to_hand) || \
    defined(choose_discard)
#error "Opening bench-alias composition requires aliases to be undefined on entry"
#endif

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

#if defined(bench_from_hand) || defined(bench_oricorio_if_useful) || \
    defined(needs_oricorio_connector) || defined(needs_tapu_connector) || \
    defined(bench_tapu_if_useful) || defined(recover_discard_to_hand)
#error "Opening bench-alias composition leaked a local alias"
#endif

// The opening layer owns the exported choose_discard alias through the adjacent
// legacy Tate body, which consumes and releases it at the historical boundary.
// Composition source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/tate_legacy_body.inc
#include "tate_legacy_body.inc"
