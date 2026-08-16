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

// Compatibility helper for existing Engine call sites. New bridges can construct
// CardContextAdapter directly, which keeps the binding object named and local while
// preserving the existing function contract during incremental .inc cleanup.
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
[[nodiscard]] inline CardContext make_card_context_adapter(
    void* engine, const CardContextAdapterCallbacks& callbacks) {
  return CardContextAdapter{engine, callbacks}.context();
}

}  // namespace sim::trace_engine_v2
