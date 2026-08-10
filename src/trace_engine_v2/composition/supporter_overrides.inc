#pragma once

// Centralize the late Supporter wrapper families behind one composition boundary.
// Their order remains behavior-significant because later wrappers observe the
// implementation exposed by earlier includes.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "gladion_overrides.inc"
#include "arven_overrides.inc"

// Supporter selector wrapper stack. Keep the selector layering at this same root
// boundary so no one-purpose include can be interleaved between priority layers.
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
