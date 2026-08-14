#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class VsSeeker final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::VsSeeker,
      .canonical_id = "xy4-109", // Card data: https://api.pokemontcg.io/v2/cards/xy4-109
      .name = "VS Seeker",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/xy4-109",
  };
};

}  // namespace sim::cards
