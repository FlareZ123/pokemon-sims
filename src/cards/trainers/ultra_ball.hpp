#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class UltraBall final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::UltraBall,
      .canonical_id = "sv1-196", // Exact Scarlet & Violet print: https://api.pokemontcg.io/v2/cards/sv1-196
      .name = "Ultra Ball",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item, // Printed Item classification: https://api.pokemontcg.io/v2/cards/sv1-196
      .source_url = "https://api.pokemontcg.io/v2/cards/sv1-196",
  };

  static constexpr int kDiscardCost = 2; // Printed cost is two other cards: https://api.pokemontcg.io/v2/cards/sv1-196
};

}  // namespace sim::cards
