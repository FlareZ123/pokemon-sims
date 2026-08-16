#ifndef SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
#define SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED

// Compatibility seam for historical include sites. Intrinsic Regidrago line
// identity is owned by the catalog/classification composition that precedes this
// seam. Keep this file as a contract check instead of forwarding to the retired
// card_classification.inc path:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#error "regidrago_line_helpers.inc requires the catalog/classification composition owner"
#endif

#endif  // SIM_TRACE_REGIDRAGO_LINE_HELPERS_INCLUDED
