#pragma once

// Canonical namespace-scope composition package for the Forretress feature.
// Runtime and scenario-extension ownership stay together under core/forretress
// while Engine is closed, preserving the historical declaration and include order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "runtime.inc"

// The historical registry is defined later in part_014c.inc. Keep its public
// declarations and extension family behind one named owner at this verified
// namespace boundary, after runtime composition and before later translation-unit
// continuation.
// Canonical registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/scenario_registry.inc
#include "scenario_registry.inc"
