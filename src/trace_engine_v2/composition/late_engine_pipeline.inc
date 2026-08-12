#pragma once

// Central ordered boundary for the post-search lock-removal and late execution
// composition. Keep these textual fragments adjacent because the first wrapper
// completes the post-search Trainer overrides before the second composes the
// remaining turn execution, scenario registry, and reporting chain.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"
