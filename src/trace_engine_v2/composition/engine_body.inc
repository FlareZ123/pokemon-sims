#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// This wrapper centralizes the textual include order without changing macro
// lifetime or member boundaries. Several legacy parts deliberately open a member
// that the next stage closes, so preserve this sequence exactly.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core engine: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_engine_body.inc
// Opening through VSTAR: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search overrides: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Turn and reporting body: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/turn_reporting_body.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

#include "core_engine_body.inc"
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#include "post_014a_overrides.inc"
#include "turn_reporting_body.inc"

// play_field_blower, run_turn, and the Garbodor scenario aliases intentionally
// remain live for part_016.inc. Release only the scenario aliases after the final
// translation-unit continuation consumes them:
// https://github.com/FlareZ123/pokemon-sims/issues/2808
#include "../part_016.inc"
#undef scenario_by_label
#undef all_scenarios
