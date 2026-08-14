#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class EarthenVessel final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::EarthenVessel,
      .canonical_id = "sv4-163", // Exact Paradox Rift Item: https://api.pokemontcg.io/v2/cards/sv4-163
      .name = "Earthen Vessel",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item, // Printed Item classification: https://api.pokemontcg.io/v2/cards/sv4-163
      .source_url = "https://api.pokemontcg.io/v2/cards/sv4-163",
  };
};

}  // namespace sim::cards
