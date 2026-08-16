#pragma once

#include <optional>
#include <string_view>

#include "../card_definition.hpp"
#include "../../rules/card_context.hpp"

namespace sim::cards {

// Reference migration for one exact Quick Ball print. The simulator strategy still
// chooses the discard and search target. This module owns the printed source-card
// lifecycle, mandatory discard, deck-search start, selected search, and shuffle.
class QuickBall final {
 public:
  using SearchTargetSelector = std::optional<Card> (*)(void*);

  struct Action {
    Card discard;
    void* search_context = nullptr;
    SearchTargetSelector choose_search_target = nullptr;
    std::string_view cost_reason = "Quick Ball cost";
    std::string_view rules_reference = "R-QB-01";
    std::string_view search_reason = "Quick Ball";
  };

  struct Resolution {
    bool played = false;
    std::optional<Card> search_target;
    bool found_target = false;
  };

  static constexpr CardDefinition definition{
      .id = Card::QuickBall,
      .canonical_id = "swsh1-179",
      .name = "Quick Ball",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh1-179",
  };

  static bool validate(const rules::CardContext& context,
                       const Action& action) {
    if (context.hand_count(Card::QuickBall) == 0) return false;

    return context.hand_count(action.discard) >=
           required_discard_copies(action.discard);
  }

  static Resolution resolve(rules::CardContext& context,
                            const Action& action) {
    if (!validate(context, action)) return {};

    // The played Item enters the discard pile as its own card lifecycle. It is not
    // the card discarded to pay Quick Ball's printed cost.
    if (!context.move_hand_to_discard(Card::QuickBall)) return {};
    if (!context.discard_from_hand(action.discard, action.cost_reason,
                                   action.rules_reference)) {
      return {};
    }

    context.begin_deck_search(action.search_reason);

    std::optional<Card> target;
    if (action.choose_search_target != nullptr) {
      target = action.choose_search_target(action.search_context);
    }
    if (target && !context.is_basic_pokemon(*target)) {
      target.reset();
    }

    const bool found = target && context.search_deck_to_hand(*target);
    context.shuffle_deck();
    return Resolution{.played = true,
                      .search_target = target,
                      .found_target = found};
  }

 private:
  static constexpr int required_discard_copies(const Card discard) {
    // Quick Ball requires "another" card, so the played copy cannot pay its own cost.
    // Exact printed effect: https://api.pokemontcg.io/v2/cards/swsh1-179
    return discard == Card::QuickBall ? 2 : 1;
  }
};

}  // namespace sim::cards
