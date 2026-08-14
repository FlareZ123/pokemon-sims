#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ForestOfVitality final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ForestOfVitality,
      .canonical_id = "me1-117", // Exact Mega Evolution Stadium: https://api.pokemontcg.io/v2/cards/me1-117
      .name = "Forest of Vitality",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Stadium, // Exact Stadium metadata: https://api.pokemontcg.io/v2/cards/me1-117
      .source_url = "https://api.pokemontcg.io/v2/cards/me1-117", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3639
  };
};

}  // namespace sim::cards
