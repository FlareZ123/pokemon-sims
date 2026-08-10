#pragma once

// Keep the Forest Seal Stone target override chain in one guarded composition
// boundary. Include order is behavior-significant because each later layer wraps
// the preceding implementation through a temporary macro alias.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(fss_target_after_search_started)
#error "FSS target composition requires its wrapper alias to be undefined on entry"
#endif

#define fss_target_after_search_started fss_target_after_search_started_issue1071_original
#include "../part_010_fss_override.inc"
#undef fss_target_after_search_started
#define fss_target_after_search_started fss_target_after_search_started_issue1735_original
#include "../part_issue_1071_fss_oricorio_treasure_decomposition_override.inc"
#undef fss_target_after_search_started
#include "../part_issue_1735_fss_blender_priority_override.inc"
#include "../part_010_attach_fss_override.inc"

#if defined(fss_target_after_search_started)
#error "FSS target composition leaked its wrapper alias"
#endif
