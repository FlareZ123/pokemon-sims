#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class RoseannesBackup final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::RoseannesBackup,
      .canonical_id = "swsh9-148", // Exact Brilliant Stars Supporter: https://api.pokemontcg.io/v2/cards/swsh9-148
      .name = "Roseanne's Backup",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh9-148", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3629
  };
};

}  // namespace sim::cards
