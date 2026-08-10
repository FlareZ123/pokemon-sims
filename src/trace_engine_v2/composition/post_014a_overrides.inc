#pragma once

// Compose the member overrides that become legal immediately after part_014a.inc
// closes run_search_items_one_step(). Keeping this boundary named avoids exposing
// four independent override families in the translation-unit root while preserving
// their established dependency order.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "serena_route_overrides.inc"
#include "burnet_fss_overrides.inc"
#include "celestial_roar_overrides.inc"
#include "final_engine_overrides.inc"
