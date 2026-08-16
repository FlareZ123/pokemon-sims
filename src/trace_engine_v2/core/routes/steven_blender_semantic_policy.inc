// Preserve the historical K1 route inclusion point while the implementation lives
// with the related K0 semantic predicate in one canonical route owner.
// Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_blender_semantic_routes.inc
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#define SIM_TRACE_STEVEN_BLENDER_K1_SECTION
#include "steven_blender_semantic_routes.inc"
#undef SIM_TRACE_STEVEN_BLENDER_K1_SECTION
