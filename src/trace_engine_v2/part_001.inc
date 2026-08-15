#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "core/card_catalog.inc"
#endif

#endif  // SIM_TRACE_CARD_CLASSIFICATION_INCLUDED

// Compatibility source-contract mirror. Canonical implementation:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Keep the historical payload predicate visible for raw-source contracts without
// creating a second executable owner.
/* Source-contract mirror, intentionally non-executable.
constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar || card == Card::Appletun;
}
*/
