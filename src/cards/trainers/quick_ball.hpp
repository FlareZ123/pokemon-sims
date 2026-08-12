#pragma once

#include <optional>

#include "../card_definition.hpp"
#include "../../rules/card_context.hpp"

namespace sim::cards {

// Reference migration for one exact Quick Ball print. The simulator strategy still
// chooses the discard and search target. This module validates and resolves only
// the printed cost/search effect through generic rules primitives.
class QuickBall final {
 public:
  struct Action {
    Card discard;
    std::optional<Card> search_target;
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
    // Quick Ball says to discard another card. A second Quick Ball is legal cost,
    // while the copy being played cannot pay for itself.
    const int copies_required = action.discard == Card::QuickBall ? 2 : 1;
    if (context.hand_count(action.discard) < copies_required) return false;
    if (action.search_target &&
        !context.is_basic_pokemon(*action.search_target)) {
      return false;
    }
    return true;
  }

  // The Engine owns the played Item's hand-to-discard lifecycle during the
  // incremental migration. Call this while that source Quick Ball is still counted
  // in hand, then move the source card using the existing Item lifecycle path.
  static bool resolve(rules::CardContext& context, const Action& action) {
    if (!validate(context, action)) return false;
    if (!context.discard_from_hand(action.discard, "Quick Ball cost",
                                   "R-QB-01")) {
      return false;
    }

    context.begin_deck_search("Quick Ball");
    if (action.search_target) {
      // A legal deck search may fail to find the selected Basic Pokemon. The
      // hidden-information/search policy remains outside this card module.
      context.search_deck_to_hand(*action.search_target);
    }
    context.shuffle_deck();
    return true;
  }
};

}  // namespace sim::cards
