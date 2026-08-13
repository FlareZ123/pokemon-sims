#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class PathToThePeak final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::PathToPeak,
      .canonical_id = "swsh6-148", // Exact card data: https://api.pokemontcg.io/v2/cards/swsh6-148
      .name = "Path to the Peak",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Stadium, // Printed Stadium classification: https://api.pokemontcg.io/v2/cards/swsh6-148
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh6-148",
  };
};

static_assert(is_trainer_kind(PathToThePeak::definition, TrainerKind::Stadium)); // Stadium procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

}  // namespace sim::cards
