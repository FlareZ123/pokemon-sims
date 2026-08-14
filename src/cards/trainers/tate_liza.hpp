#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class TateLiza final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::TateLiza,
      .canonical_id = "sm7-148", // Exact card data: https://api.pokemontcg.io/v2/cards/sm7-148
      .name = "Tate & Liza",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Printed Supporter classification: https://api.pokemontcg.io/v2/cards/sm7-148
      .source_url = "https://api.pokemontcg.io/v2/cards/sm7-148", // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3562
  };
};

}  // namespace sim::cards
