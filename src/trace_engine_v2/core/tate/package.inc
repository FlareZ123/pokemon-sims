#pragma once

// Canonical Tate/provenance override package. Keep these member fragments in their
// established textual order while the remaining root-level source-contract mirrors migrate.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

// 1. Discard/recovery provenance owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/discard_recovery_provenance.inc
#include "discard_recovery_provenance.inc"

// 2. Proactive Tate attachment owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/attachment_policy.inc
#include "attachment_policy.inc"

// 3. Tate action owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/action_policy.inc
#include "action_policy.inc"