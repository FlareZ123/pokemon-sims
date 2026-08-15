// Middle composition stage: banked-Tapu replacement followed by lock-removal wiring.
// This file is textually included from engine_body.inc at the historical boundary.
// The named substages preserve the exact macro setup/include/teardown order and
// keep each temporary-alias contract local to its composition owner.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

#include "banked_tapu_policy_stage.inc"
#include "lock_removal_policy_stage.inc"
