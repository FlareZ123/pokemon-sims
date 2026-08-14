#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Powerglass final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Powerglass,
      .canonical_id = "sv6pt5-63", // Card data: https://api.pokemontcg.io/v2/cards/sv6pt5-63
      .name = "Powerglass",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Tool, // Exact Pokémon Tool print: https://api.pokemontcg.io/v2/cards/sv6pt5-63
      .source_url = "https://api.pokemontcg.io/v2/cards/sv6pt5-63", // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3553
  };
};

}  // namespace sim::cards
