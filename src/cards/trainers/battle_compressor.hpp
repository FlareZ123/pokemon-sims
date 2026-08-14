#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class BattleCompressor final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::BattleCompressor,
      .canonical_id = "xy4-92", // Card data: https://api.pokemontcg.io/v2/cards/xy4-92
      .name = "Battle Compressor Team Flare Gear",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/xy4-92",
  };
};

}  // namespace sim::cards
