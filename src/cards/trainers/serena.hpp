#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Serena final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Serena,
      .canonical_id = "swsh12-164", // Exact Silver Tempest print: https://api.pokemontcg.io/v2/cards/swsh12-164
      .name = "Serena",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact Supporter subtype: https://api.pokemontcg.io/v2/cards/swsh12-164
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12-164", // Recovery issue: https://github.com/FlareZ123/pokemon-sims/issues/3585
  };
};

}  // namespace sim::cards
