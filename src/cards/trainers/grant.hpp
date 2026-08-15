#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Grant final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Grant,
      .canonical_id = "swsh10-144", // Exact Astral Radiance print: https://api.pokemontcg.io/v2/cards/swsh10-144
      .name = "Grant",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact Supporter subtype: https://api.pokemontcg.io/v2/cards/swsh10-144
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh10-144", // Architecture owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  };
};

}  // namespace sim::cards
