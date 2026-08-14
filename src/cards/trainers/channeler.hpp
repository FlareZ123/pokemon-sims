#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Channeler final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Channeler,
      .canonical_id = "sm11-190", // Exact Unified Minds Supporter: https://api.pokemontcg.io/v2/cards/sm11-190
      .name = "Channeler",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter,
      .source_url = "https://api.pokemontcg.io/v2/cards/sm11-190", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3624
  };
};

}  // namespace sim::cards
