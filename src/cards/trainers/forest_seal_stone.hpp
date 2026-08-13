#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ForestSealStone final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ForestSealStone,
      .canonical_id = "swsh12-156", // Card data: https://api.pokemontcg.io/v2/cards/swsh12-156
      .name = "Forest Seal Stone",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Tool, // Pokémon Tool: https://api.pokemontcg.io/v2/cards/swsh12-156
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12-156", // https://github.com/FlareZ123/pokemon-sims/issues/3505
  };
};

}  // namespace sim::cards
