#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ForestSealStone final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ForestSealStone,
      .canonical_id = "swsh12-156", // Card data: https://api.pokemontcg.io/v2/cards/swsh12-156
      .name = "Forest Seal Stone",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Tool, // Exact Pokémon Tool print: https://api.pokemontcg.io/v2/cards/swsh12-156
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12-156", // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3612
  };
};

}  // namespace sim::cards
