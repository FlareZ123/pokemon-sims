#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class BrilliantBlender final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::BrilliantBlender,
      .canonical_id = "sv8-164", // Exact card data: https://api.pokemontcg.io/v2/cards/sv8-164
      .name = "Brilliant Blender",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item, // Printed Item classification: https://api.pokemontcg.io/v2/cards/sv8-164
      .ace_spec = true, // Printed ACE SPEC classification/rule: https://api.pokemontcg.io/v2/cards/sv8-164
      .source_url = "https://api.pokemontcg.io/v2/cards/sv8-164",
  };
};

}  // namespace sim::cards
