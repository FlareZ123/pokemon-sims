#pragma once

// Legacy source-link shim. Reusable Wonder Tag route policy has one implementation
// under core/, so historical fixtures keep this path without duplicating logic.
// Canonical policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tapu_wonder_tag_route_policy.inc
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/2270
#include "core/tapu_wonder_tag_route_policy.inc"
