#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Klara final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Klara,
      .canonical_id = "swsh6-145", // Exact Chilling Reign Supporter: https://api.pokemontcg.io/v2/cards/swsh6-145
      .name = "Klara",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh6-145", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3625
  };
};

}  // namespace sim::cards
