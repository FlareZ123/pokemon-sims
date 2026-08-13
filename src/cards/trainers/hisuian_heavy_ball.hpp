#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class HisuianHeavyBall final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::HisuianHeavyBall,
      .canonical_id = "swsh10-146", // Exact card data: https://api.pokemontcg.io/v2/cards/swsh10-146
      .name = "Hisuian Heavy Ball",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item, // Printed Item classification: https://api.pokemontcg.io/v2/cards/swsh10-146
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh10-146",
  };
};

}  // namespace sim::cards
