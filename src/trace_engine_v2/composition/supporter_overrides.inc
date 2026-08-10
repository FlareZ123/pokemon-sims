#pragma once

// Centralize the late Supporter wrapper families behind one composition boundary.
// Their order remains behavior-significant because later wrappers observe the
// implementation exposed by earlier includes.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "supporter_gladion_overrides.inc"
#include "supporter_arven_overrides.inc"
#include "supporter_selector_overrides.inc"
