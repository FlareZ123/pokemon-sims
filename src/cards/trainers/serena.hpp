#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Serena final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Serena,
      .canonical_id = "swsh12-164", // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-164
      .name = "Serena",
      .kind = CardKind::Trainer, // Trainer identity: https://api.pokemontcg.io/v2/cards/swsh12-164
      .trainer_kind = TrainerKind::Supporter, // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/swsh12-164
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12-164", // Cleanup issue: https://github.com/FlareZ123/pokemon-sims/issues/3585
  };
};

}  // namespace sim::cards
