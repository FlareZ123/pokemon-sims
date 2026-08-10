#pragma once

// Keep the post-evolution discard provenance and Tate wrapper family behind one
// composition boundary. Include order remains behavior-significant because each
// later wrapper observes the implementation exposed by the preceding layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_discard_recovery_provenance_override.inc"
#include "../part_tate_blender_attachment_override.inc"
#include "../part_tate_blender_tate_override.inc"
