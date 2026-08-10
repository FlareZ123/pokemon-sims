#pragma once

// Keep the Legacy Star policy family behind one stable composition boundary while
// the projection-specific macro chain lives in its focused helper include.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#include "legacy_star_projection_chain.inc"
