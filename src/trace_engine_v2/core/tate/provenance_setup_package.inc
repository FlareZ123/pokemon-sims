#pragma once

// Canonical Tate pre-action subpackage. Keep discard/recovery provenance before
// proactive attachment so the Engine member declarations retain their established
// textual order. This file only centralizes composition; gameplay policy remains in
// the two canonical owners below.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md

// Discard/recovery provenance owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/discard_recovery_provenance.inc
#include "discard_recovery_provenance.inc"

// Proactive attachment owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/attachment_policy.inc
#include "attachment_policy.inc"
