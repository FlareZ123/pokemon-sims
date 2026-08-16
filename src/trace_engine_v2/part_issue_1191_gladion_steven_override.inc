// Historical Gladion/Steven/Treasure compatibility seam at the post-search composition boundary.
// Canonical Steven route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/gladion_steven_route_policy.inc
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if !defined(play_gladion)
#error "Gladion/Steven composition requires the parent play_gladion alias"
#endif
#include "core/routes/gladion_steven_route_policy.inc"
#if !defined(play_gladion)
#error "Gladion/Steven composition unexpectedly released the parent play_gladion alias"
#endif
