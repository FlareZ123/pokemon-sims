#pragma once

#include "../../../rules/card_context.hpp"

namespace sim::trace_engine_v2 {

using CardContext = rules::CardContext;
using CardContextAdapterCallbacks = CardContext::Callbacks;

// Canonical adapter between Engine internals and reusable card effects. Each
// migrated resolver supplies callbacks that delegate to the Engine's existing zone,
// knowledge, trace, and shuffle helpers. Keeping construction here prevents card
// modules from depending on trace_engine_v2 implementation details while the rules
// layer remains the single owner of the callback bundle shape.
// Cleanup ownership: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
class CardContextAdapter final {
 public:
  [[nodiscard]] static CardContext make(
      void* engine, const CardContextAdapterCallbacks& callbacks) {
    return CardContext{engine, callbacks};
  }
};

// Preserve the established free-function seam while centralizing construction in
// the named adapter class. Existing card resolvers can migrate independently.
[[nodiscard]] inline CardContext make_card_context_adapter(
    void* engine, const CardContextAdapterCallbacks& callbacks) {
  return CardContextAdapter::make(engine, callbacks);
}

}  // namespace sim::trace_engine_v2
