#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Dawn final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Dawn,
      .canonical_id = "me2-87", // Exact card data: https://api.pokemontcg.io/v2/cards/me2-87
      .name = "Dawn",
      .kind = CardKind::Trainer, // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact subtype and printed one-Supporter reminder: https://api.pokemontcg.io/v2/cards/me2-87
      .source_url = "https://api.pokemontcg.io/v2/cards/me2-87", // Cleanup issue: https://github.com/FlareZ123/pokemon-sims/issues/3560
  };
};

}  // namespace sim::cards
