#pragma once

#include "../../../rules/card_context.hpp"

namespace sim::trace_engine_v2 {

using CardContext = rules::CardContext;
using CardContextAdapterCallbacks = CardContext::Callbacks;

// Own the trace-engine side of CardContext construction in one small adapter object.
// The rules layer still owns the callback contract; this type only binds one Engine
// instance to that already-defined callback bundle.
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
class CardContextAdapter final {
 public:
  using Callbacks = CardContextAdapterCallbacks;

  CardContextAdapter(void* engine, const Callbacks& callbacks)
      : engine_(engine), callbacks_(callbacks) {}

  [[nodiscard]] CardContext context() const {
    return CardContext{engine_, callbacks_};
  }

 private:
  void* engine_;
  Callbacks callbacks_;
};

// Canonical adapter between Engine internals and reusable card effects. Each
// migrated resolver supplies callbacks that delegate to the Engine's existing zone,
// knowledge, trace, and shuffle helpers. Keeping construction here prevents card
// modules from depending on trace_engine_v2 implementation details while the rules
// layer remains the single owner of the callback bundle shape.
[[nodiscard]] inline CardContext make_card_context_adapter(
    void* engine, const CardContextAdapterCallbacks& callbacks) {
  return CardContext{engine, callbacks};
}

}  // namespace sim::trace_engine_v2
