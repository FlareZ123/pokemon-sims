#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Guzma final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Guzma,
      .canonical_id = "sm3-115",  // Exact Burning Shadows print: https://api.pokemontcg.io/v2/cards/sm3-115
      .name = "Guzma",
      .kind = CardKind::Trainer,  // Trainer identity: https://api.pokemontcg.io/v2/cards/sm3-115
      .trainer_kind = TrainerKind::Supporter,  // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/sm3-115 ; procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .source_url = "https://api.pokemontcg.io/v2/cards/sm3-115",  // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/issues/3618
  };
};

}  // namespace sim::cards
