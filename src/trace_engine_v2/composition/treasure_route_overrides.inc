#pragma once

// Compatibility shim for the historical post_014a include order.
// `earthen_vessel_overrides.inc` now composes this Treasure family immediately
// after the Earthen Vessel chain, at the same adjacent composition boundary.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
