#pragma once

// Compatibility seam for the post-search Arven composition boundary. Intrinsic
// Regidrago line identity is already owned by the canonical card catalog before
// this shim is reached, so do not reopen a retired classification implementation.
// Canonical classifier owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Canonical composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#error "Regidrago line helpers require canonical card classification to be composed first"
#endif
