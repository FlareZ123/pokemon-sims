#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class EvolutionIncense final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::EvolutionIncense,
      .canonical_id = "swsh1-163", // Card data: https://api.pokemontcg.io/v2/cards/swsh1-163
      .name = "Evolution Incense",
      .kind = CardKind::Trainer, // Rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh1-163", // Issue: https://github.com/FlareZ123/pokemon-sims/issues/3471
  };
};

}  // namespace sim::cards
