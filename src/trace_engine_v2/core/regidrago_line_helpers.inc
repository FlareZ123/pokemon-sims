#ifndef SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
#define SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED

// Compatibility seam for historical include sites. Intrinsic Regidrago line
// identity is owned by the shared card-classification module so route policy
// reuses the same predicate as other Pokémon identity queries:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "card_classification.inc"
#endif

#endif  // SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED