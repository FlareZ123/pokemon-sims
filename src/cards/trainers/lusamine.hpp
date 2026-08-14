#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Lusamine final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Lusamine,
      .canonical_id = "sm4-96", // Exact Crimson Invasion Supporter: https://api.pokemontcg.io/v2/cards/sm4-96
      .name = "Lusamine",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter,
      .source_url = "https://api.pokemontcg.io/v2/cards/sm4-96", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3619
  };
};

}  // namespace sim::cards
