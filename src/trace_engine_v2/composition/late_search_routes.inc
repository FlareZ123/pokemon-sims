#pragma once

// Ordered late-search composition spine. The first unit owns empty-deck search
// wrappers and shared target policies; the second owns the post-search connector
// chain through the Quick Ball tail. Both units preserve the historical textual
// include order and close every wrapper alias they open.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Empty-deck unit: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/empty_deck_search_stack.inc
// Post-search unit: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_connector_stack.inc

#include "empty_deck_search_stack.inc"
#include "post_search_connector_stack.inc"
