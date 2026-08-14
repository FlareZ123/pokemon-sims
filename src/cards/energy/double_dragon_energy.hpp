#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class DoubleDragonEnergy final {
 public:
  // Exact Roaring Skies Special Energy print: https://api.pokemontcg.io/v2/cards/xy6-97
  // Printed Dragon-only attachment/provision behavior remains in Engine policy until a later behavior migration.
  static constexpr CardDefinition definition{
      .id = Card::DoubleDragonEnergy,
      .canonical_id = "xy6-97",
      .name = "Double Dragon Energy",
      .kind = CardKind::Energy,
      .basic_energy = false,
      .source_url = "https://api.pokemontcg.io/v2/cards/xy6-97",
  };
};

}  // namespace sim::cards
