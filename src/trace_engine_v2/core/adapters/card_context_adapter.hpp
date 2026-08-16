#pragma once

#include "../../../rules/card_context.hpp"

namespace sim::trace_engine_v2 {

using CardContext = rules::CardContext;

// Canonical adapter between Engine internals and reusable card effects. Each
// migrated resolver supplies callbacks that delegate to the Engine's existing zone,
// knowledge, trace, and shuffle helpers. Keeping construction here prevents card
// modules from depending on trace_engine_v2 implementation details.
[[nodiscard]] inline CardContext make_card_context_adapter(
    void* engine,
    const CardContext::HandCountFn hand_count,
    const CardContext::MoveHandToDiscardFn move_hand_to_discard,
    const CardContext::DiscardFromHandFn discard_from_hand,
    const CardContext::SearchDeckToHandFn search_deck_to_hand,
    const CardContext::ShuffleDeckFn shuffle_deck,
    const CardContext::IsBasicPokemonFn is_basic_pokemon,
    const CardContext::BeginDeckSearchFn begin_deck_search) {
  return CardContext{engine,
                     hand_count,
                     move_hand_to_discard,
                     discard_from_hand,
                     search_deck_to_hand,
                     shuffle_deck,
                     is_basic_pokemon,
                     begin_deck_search};
}

}  // namespace sim::trace_engine_v2
