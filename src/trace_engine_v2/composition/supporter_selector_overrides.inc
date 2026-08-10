#pragma once

// Supporter selector wrapper stack. Keep selector layering in one ordered unit so
// callers cannot accidentally interleave unrelated wrappers between these layers.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#ifdef choose_supporter
#error "Supporter selector composition requires choose_supporter to be undefined on entry"
#endif

#define choose_supporter choose_supporter_original
#include "../part_014b.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_turo_original
#include "../part_team_yell_vstar_override.inc"
#undef choose_supporter

#include "../part_roseanne_multimode_override.inc"

#define choose_supporter choose_supporter_klara_original
#include "../part_turo_oricorio_override.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_issue1070_original
#include "../part_klara_recovery_override.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_issue1209_original
#include "../part_issue_1070_tate_after_vstar_search_override.inc"
#undef choose_supporter

#ifdef choose_supporter
#error "Supporter selector composition leaked choose_supporter"
#endif
