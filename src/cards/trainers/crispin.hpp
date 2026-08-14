#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Crispin final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Crispin,
      .canonical_id = "sv7-133", // Exact Stellar Crown print: https://api.pokemontcg.io/v2/cards/sv7-133
      .name = "Crispin",
      .kind = CardKind::Trainer, // Trainer identity: https://api.pokemontcg.io/v2/cards/sv7-133
      .trainer_kind = TrainerKind::Supporter, // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/sv7-133
      .source_url = "https://api.pokemontcg.io/v2/cards/sv7-133", // Cleanup issue: https://github.com/FlareZ123/pokemon-sims/issues/3580
  };
};

}  // namespace sim::cards
