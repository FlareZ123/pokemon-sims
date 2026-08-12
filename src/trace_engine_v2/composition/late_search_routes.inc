#pragma once

// Consolidated late-search route composition. Keep these units in this order because
// later wrappers consume member implementations exposed by the preceding layer.
// Each unit owns its alias-entry and alias-exit contracts.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#include "empty_deck_search_routes.inc"
#include "post_search_connector_routes.inc"
#include "quick_ball_tail.inc"
