#pragma once

// Keep post-evolution discard provenance and Tate handling behind one ordered
// composition boundary, then delegate the Item-search wrapper family separately.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_discard_recovery_provenance_override.inc"
#include "../part_tate_blender_attachment_override.inc"
#include "../part_tate_blender_tate_override.inc"
#include "tate_search_item_overrides.inc"

// play_ultra_ball intentionally remains exported until the root translation unit
// finishes the legacy Item wrapper layer and releases it before late overrides.
