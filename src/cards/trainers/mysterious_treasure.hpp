#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class MysteriousTreasure final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::MysteriousTreasure,
      .canonical_id = "sm6-113", // Exact card data: https://api.pokemontcg.io/v2/cards/sm6-113
      .name = "Mysterious Treasure",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item, // Printed Item classification: https://api.pokemontcg.io/v2/cards/sm6-113
      .source_url = "https://api.pokemontcg.io/v2/cards/sm6-113",
  };
};

}  // namespace sim::cards
