#pragma once

// Canonical Tate/provenance override package. Keep these member fragments in their
// established textual order while the remaining root-level part_* owners migrate.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Canonical discard/recovery provenance owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/discard_recovery_provenance.inc
// Canonical Tate attachment owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/attachment_policy.inc
// Canonical Tate action composition seam: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/action_policy.inc
#include "discard_recovery_provenance.inc"
#include "attachment_policy.inc"
#include "action_policy.inc"
