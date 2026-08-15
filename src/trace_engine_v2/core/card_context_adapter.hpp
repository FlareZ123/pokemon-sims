#pragma once

#include "../../rules/card_context.hpp"

namespace sim::trace_engine_v2 {

using CardContext = rules::CardContext;

// Keep the callback bundle at the trace-engine boundary so card resolvers depend on
// rules::CardContext rather than Engine implementation details.
// Cleanup architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
// Context contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/rules/card_context.hpp
struct CardContextCallbacks {
  CardContext::HandCountFn hand_count;
  CardContext::MoveHandToDiscardFn move_hand_to_discard;
  CardContext::DiscardFromHandFn discard_from_hand;
  CardContext::SearchDeckToHandFn search_deck_to_hand;
  CardContext::ShuffleDeckFn shuffle_deck;
  CardContext::IsBasicPokemonFn is_basic_pokemon;
  CardContext::BeginDeckSearchFn begin_deck_search;
};

inline CardContext make_card_context_adapter(
    void* engine, const CardContextCallbacks& callbacks) {
  return CardContext{engine, callbacks.hand_count, callbacks.move_hand_to_discard,
                     callbacks.discard_from_hand, callbacks.search_deck_to_hand,
                     callbacks.shuffle_deck, callbacks.is_basic_pokemon,
                     callbacks.begin_deck_search};
}

// Transitional seam between Engine internals and reusable card effects. Each
// migrated resolver supplies callbacks that delegate to the Engine's existing zone,
// knowledge, trace, and shuffle helpers. Keeping construction here prevents card
// modules from depending on trace_engine_v2 implementation details.
inline CardContext make_card_context_adapter(
    void* engine, const CardContext::HandCountFn hand_count,
    const CardContext::MoveHandToDiscardFn move_hand_to_discard,
    const CardContext::DiscardFromHandFn discard_from_hand,
    const CardContext::SearchDeckToHandFn search_deck_to_hand,
    const CardContext::ShuffleDeckFn shuffle_deck,
    const CardContext::IsBasicPokemonFn is_basic_pokemon,
    const CardContext::BeginDeckSearchFn begin_deck_search) {
  return CardContext{engine, hand_count, move_hand_to_discard,
                     discard_from_hand, search_deck_to_hand,
                     shuffle_deck, is_basic_pokemon,
                     begin_deck_search};
}

}  // namespace sim::trace_engine_v2