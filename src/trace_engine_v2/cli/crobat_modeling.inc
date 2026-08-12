#pragma once

// Crobat modeling is a CLI-only experimental surface kept outside the main
// translation-unit tail so part_016 remains focused on CLI parsing/dispatch.
// Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

namespace sim {

#include "crobat_modeling_decks.inc"
#include "crobat_modeling_matrix.inc"

}  // namespace sim
