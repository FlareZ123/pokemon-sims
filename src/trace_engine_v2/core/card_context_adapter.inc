#pragma once

#include "../../rules/card_context.hpp"

namespace sim::trace_engine_v2 {

// Transitional seam between Engine internals and reusable card effects. Each
// migrated resolver supplies callbacks that delegate to the Engine's existing zone,
// knowledge, trace, and shuffle helpers. Keeping construction here prevents card
// modules from depending on trace_engine_v2 implementation details.
inline rules::CardContext make_card_context_adapter(
    void* engine, const rules::CardContext::HandCountFn hand_count,
    const rules::CardContext::MoveHandToDiscardFn move_hand_to_discard,
    const rules::CardContext::DiscardFromHandFn discard_from_hand,
    const rules::CardContext::SearchDeckToHandFn search_deck_to_hand,
    const rules::CardContext::ShuffleDeckFn shuffle_deck,
    const rules::CardContext::IsBasicPokemonFn is_basic_pokemon,
    const rules::CardContext::BeginDeckSearchFn begin_deck_search) {
  return rules::CardContext{engine, hand_count, move_hand_to_discard,
                            discard_from_hand, search_deck_to_hand,
                            shuffle_deck, is_basic_pokemon,
                            begin_deck_search};
}

}  // namespace sim::trace_engine_v2
