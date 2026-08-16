#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED

// `part_000.inc` owns SIM_TRACE_CARD_CATALOG_INCLUDED; delegate its guard here.
// Canonical catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
#include "part_000.inc"
// Keep this compatibility shim focused on classification-level delegation.

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