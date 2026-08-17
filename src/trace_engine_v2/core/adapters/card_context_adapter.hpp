#pragma once

#include "../../../rules/card_context.hpp"

namespace sim::trace_engine_v2 {

using CardContext = rules::CardContext;

// Canonical adapter between Engine internals and reusable card effects. Each
// migrated resolver supplies callbacks that delegate to the Engine's existing zone,
// knowledge, trace, and shuffle helpers. Keeping construction here prevents card
// modules from depending on trace_engine_v2 implementation details while the rules
// layer remains the single owner of the callback bundle shape.
// Cleanup ownership: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
class CardContextAdapter final {
 public:
  using Callbacks = CardContext::Callbacks;

  [[nodiscard]] static CardContext make(
      void* engine, const Callbacks& callbacks) {
    return CardContext{engine, callbacks};
  }
};

// Preserve the established namespace-level type and free-function seams while
// centralizing adapter-specific aliases and construction on CardContextAdapter.
using CardContextAdapterCallbacks = CardContextAdapter::Callbacks;

[[nodiscard]] inline CardContext make_card_context_adapter(
    void* engine, const CardContextAdapterCallbacks& callbacks) {
  return CardContextAdapter::make(engine, callbacks);
}

}  // namespace sim::trace_engine_v2
