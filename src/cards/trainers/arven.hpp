#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Arven final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Arven,
      .canonical_id = "sv1-166", // Exact card data: https://api.pokemontcg.io/v2/cards/sv1-166
      .name = "Arven",
      .kind = CardKind::Trainer, // Trainer identity: https://api.pokemontcg.io/v2/cards/sv1-166
      .trainer_kind = TrainerKind::Supporter, // Printed Supporter subtype and one-Supporter reminder: https://api.pokemontcg.io/v2/cards/sv1-166
      .source_url = "https://api.pokemontcg.io/v2/cards/sv1-166", // Cleanup issue: https://github.com/FlareZ123/pokemon-sims/issues/3574
  };
};

}  // namespace sim::cards
