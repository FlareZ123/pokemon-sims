#pragma once

// Compatibility forwarding include. The Wonder Tag -> Arven -> Quick Ball -> Latias
// route policy now lives with reusable connector policy under core/ while historical
// composition sites can keep this stable path.
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/2270
// Central policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tapu_wonder_tag_route_policy.inc
#include "core/tapu_wonder_tag_route_policy.inc"
