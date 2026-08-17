#pragma once

// Canonical Tate/provenance override package. Keep the pre-action policy package
// before the action owner so the established Engine member declaration order is
// preserved while related provenance/setup fragments share one internal boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

// 1. Tate provenance/setup subpackage:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/provenance_setup_package.inc
#include "provenance_setup_package.inc"

// 2. Tate action owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/action_policy.inc
#include "action_policy.inc"
