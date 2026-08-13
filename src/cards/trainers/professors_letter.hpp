#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ProfessorsLetter final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ProfessorsLetter,
      .canonical_id = "xy1-123", // Exact card data: https://api.pokemontcg.io/v2/cards/xy1-123
      .name = "Professor's Letter",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item, // Printed Item classification: https://api.pokemontcg.io/v2/cards/xy1-123
      .source_url = "https://api.pokemontcg.io/v2/cards/xy1-123",
  };
};

}  // namespace sim::cards
