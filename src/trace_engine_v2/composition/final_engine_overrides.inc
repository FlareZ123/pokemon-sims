#pragma once

// Keep the late Engine policy fragments in their established dependency order so
// regidrago_sim.cpp owns one composition boundary instead of four scattered includes.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "legacy_star_overrides.inc"
#include "supporter_overrides.inc"
#include "search_item_overrides.inc"
#include "secret_box_trace_overrides.inc"
