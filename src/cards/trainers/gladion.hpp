#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Gladion final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Gladion,
      .canonical_id = "sm4-95",  // Exact Crimson Invasion print: https://api.pokemontcg.io/v2/cards/sm4-95
      .name = "Gladion",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Supporter,  // Printed Supporter classification: https://api.pokemontcg.io/v2/cards/sm4-95
      .source_url = "https://api.pokemontcg.io/v2/cards/sm4-95",  // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3604
  };
};

}  // namespace sim::cards
