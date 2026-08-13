#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class BattleVipPass final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::BattleVipPass,
      .canonical_id = "swsh8-225", // Card data: https://api.pokemontcg.io/v2/cards/swsh8-225
      .name = "Battle VIP Pass",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh8-225",
  };
};

}  // namespace sim::cards
