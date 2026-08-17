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

    // Quick Ball says to discard another card. A second Quick Ball is legal cost,
    // while the copy being played cannot pay for itself.
    // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179
    const int copies_required = action.discard == Card::QuickBall ? 2 : 1;
    return context.hand_count(action.discard) >= copies_required;
  }

  static Resolution resolve(rules::CardContext& context,
                            const Action& action) {
    if (!validate(context, action)) return {};

    // Follow Quick Ball's printed discard cost before its search.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    if (!context.discard_from_hand(action.discard, action.cost_reason,
                                   action.rules_reference)) {
      return {};
    }

    // Playing the Item removes that source copy from ordinary hand visibility while
    // its effect is resolving. B-01 discards the Item only after it has been used.
    // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed resolving-source defect: https://github.com/FlareZ123/pokemon-sims/issues/4293
    if (!context.move_hand_to_resolving(Card::QuickBall)) return {};

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

    // B-01 step 4 moves the Item from its resolving-source zone to discard after
    // Quick Ball's search and required shuffle have completed.
    // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Confirmed resolving-source defect: https://github.com/FlareZ123/pokemon-sims/issues/4293
    if (!context.move_resolving_to_discard(Card::QuickBall)) return {};

    return Resolution{.played = true,
                      .search_target = target,
                      .found_target = found};
  }
};

}  // namespace sim::cards