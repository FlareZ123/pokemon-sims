#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ChaoticSwell final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ChaoticSwell,
      .canonical_id = "sm12-187", // Card data: https://api.pokemontcg.io/v2/cards/sm12-187
      .name = "Chaotic Swell",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Stadium, // Exact Stadium print: https://api.pokemontcg.io/v2/cards/sm12-187
      .source_url = "https://api.pokemontcg.io/v2/cards/sm12-187", // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3563
  };
};

}  // namespace sim::cards
