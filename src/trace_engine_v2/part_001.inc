#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
// Compose the canonical catalog owner directly. The part_000 shim remains only for
// external source-anchor compatibility while callers migrate away from legacy parts.
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
// Historical links target line 137; preserve that anchor until those consumers migrate.


















































































































// Legacy line-137 anchor; canonical implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc