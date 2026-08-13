#pragma once

#include <array>
#include <optional>
#include <string_view>

#include "../card_definition.hpp"
#include "../../rules/card_context.hpp"

namespace sim::cards {

// Exact SM12 229/236 Supporter resolver. Engine strategy selects whether to pay the
// optional two-card cost and which legal categories to search. This class owns the
// printed source-card lifecycle, cost timing, category validation, deck inspection,
// selected searches, and the single final shuffle.
class GuzmaHala final {
 public:
  struct SearchTargets {
    std::optional<Card> stadium;
    std::optional<Card> tool;
    std::optional<Card> special_energy;
  };

  using SearchTargetSelector = SearchTargets (*)(void*);

  struct Action {
    bool use_bonus = false;
    std::array<Card, 2> discards{};
    void* search_context = nullptr;
    SearchTargetSelector choose_search_targets = nullptr;
    std::string_view cost_reason = "Guzma & Hala optional cost";
    std::string_view rules_reference = "R-GUZMA-HALA-01";
    std::string_view search_reason = "Guzma & Hala";
  };

  struct Resolution {
    bool played = false;
    SearchTargets search_targets;
    bool found_stadium = false;
    bool found_tool = false;
    bool found_special_energy = false;
  };

  static constexpr CardDefinition definition{
      .id = Card::GuzmaHala,
      .canonical_id = "sm12-229",
      .name = "Guzma & Hala",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Supporter,
      .source_url = "https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/",
  };

  static bool validate(const rules::CardContext& context,
                       const Action& action) {
    if (context.hand_count(Card::GuzmaHala) == 0) return false;
    if (!action.use_bonus) return true;

    // The optional cost is "discard 2 other cards from your hand." Another physical
    // Guzma & Hala may be discarded, while the copy being played cannot pay its own
    // cost. Count physical copies for repeated identities rather than comparing only
    // enum values. Card: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    const auto payable = [&context, &action](const Card card) {
      int required = card == Card::GuzmaHala ? 1 : 0;
      for (const Card discard : action.discards) {
        if (discard == card) ++required;
      }
      return context.hand_count(card) >= required;
    };
    return payable(action.discards[0]) && payable(action.discards[1]);
  }

  static Resolution resolve(rules::CardContext& context,
                            const Action& action) {
    if (!validate(context, action)) return {};

    if (!context.move_hand_to_discard(Card::GuzmaHala)) return {};
    if (action.use_bonus) {
      // TPCi's Cosmic Eclipse ruling says the two-card optional cost is paid before
      // the Stadium search. When the bonus is paid, all three category searches may
      // be performed in the same deck inspection and followed by one shuffle.
      // https://compendium.pokegym.net/category/5-trainers/guzma-and-hala/
      if (!context.discard_from_hand(action.discards[0], action.cost_reason,
                                     action.rules_reference) ||
          !context.discard_from_hand(action.discards[1], action.cost_reason,
                                     action.rules_reference)) {
        return {};
      }
    }

    context.begin_deck_search(action.search_reason);
    SearchTargets targets;
    if (action.choose_search_targets != nullptr) {
      targets = action.choose_search_targets(action.search_context);
    }

    // Validate every strategy-selected category inside the card resolver. The bonus
    // Tool and Special Energy searches exist only when the two-card cost was paid.
    // Card: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    if (targets.stadium && !context.is_stadium(*targets.stadium)) {
      targets.stadium.reset();
    }
    if (!action.use_bonus) {
      targets.tool.reset();
      targets.special_energy.reset();
    } else {
      if (targets.tool && !context.is_pokemon_tool(*targets.tool)) {
        targets.tool.reset();
      }
      if (targets.special_energy &&
          !context.is_special_energy(*targets.special_energy)) {
        targets.special_energy.reset();
      }
    }

    const bool found_stadium =
        targets.stadium && context.search_deck_to_hand(*targets.stadium);
    const bool found_tool = targets.tool && context.search_deck_to_hand(*targets.tool);
    const bool found_special_energy =
        targets.special_energy && context.search_deck_to_hand(*targets.special_energy);
    context.shuffle_deck();

    return Resolution{.played = true,
                      .search_targets = targets,
                      .found_stadium = found_stadium,
                      .found_tool = found_tool,
                      .found_special_energy = found_special_energy};
  }
};

}  // namespace sim::cards
