#pragma once

// Middle composition stage: banked-Tapu replacement followed by lock-removal wiring.
// This file is textually included from engine_body.inc at the historical boundary.
// Keep this named sequencer single-composed just like the parent engine body while
// the named substages own their exact macro setup/include/teardown boundaries.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Canonical composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

#include "banked_tapu_policy_stage.inc"
#include "lock_removal_policy_stage.inc"
