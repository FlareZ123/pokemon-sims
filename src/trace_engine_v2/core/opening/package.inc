#pragma once

// Canonical early Engine foundation package. Preserve this textual order because
// deck knowledge consumes the temporary begin_turn alias, then turn lifecycle and
// the connector/payload policies continue at the same Engine member boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// K0/K1 knowledge owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_knowledge.inc
// Turn lifecycle owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/turn_lifecycle.inc
// Tapu connector owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tapu_connector_policy.inc
// Dragon payload owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
#include "../deck_knowledge.inc"
#undef begin_turn
#include "../turn_lifecycle.inc"
#include "../tapu_connector_policy.inc"
#include "../payload_hand_policy.inc"
