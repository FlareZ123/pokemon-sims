#pragma once

// Ordered tail composition after part_014a has closed the opening search member.
// Keep lock-removal policy ahead of late execution/reporting because both are
// textual class-member fragments whose ordering is behavior-significant.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"
