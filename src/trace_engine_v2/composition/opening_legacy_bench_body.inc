#pragma once

// Preserve the legacy part_006 Bench, connector, recovery, and discard aliases in
// one named composition boundary. choose_discard intentionally remains exported
// for the Tate continuation that follows this include.
// Historical split source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_006.inc
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
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
