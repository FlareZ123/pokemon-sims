#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class WishfulBaton final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::WishfulBaton,
      .canonical_id = "sm3-128", // Card data: https://api.pokemontcg.io/v2/cards/sm3-128
      .name = "Wishful Baton",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Tool, // Exact Pokémon Tool print: https://api.pokemontcg.io/v2/cards/sm3-128
      .source_url = "https://api.pokemontcg.io/v2/cards/sm3-128", // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3631
  };
};

}  // namespace sim::cards
