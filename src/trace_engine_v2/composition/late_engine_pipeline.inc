#pragma once

// Central ordered boundary for the post-search lock-removal and late execution
// composition. Keep these textual fragments adjacent because the first wrapper
// completes the post-search Trainer overrides before the second composes the
// remaining turn execution, scenario registry, and reporting chain.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc
#if defined(play_chaotic_swell) || defined(play_field_blower) || \
    defined(run_turn) || defined(hand_count) || defined(play_items_until_stable)
#error "Late engine pipeline requires temporary execution aliases to be undefined on entry"
#endif

#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

#if defined(play_chaotic_swell) || defined(play_field_blower) || \
    defined(run_turn) || defined(hand_count) || defined(play_items_until_stable)
#error "Late engine pipeline leaked a temporary execution alias"
#endif
