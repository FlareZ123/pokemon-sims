#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class Arven final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Arven,
      .canonical_id = "sv1-166", // Exact Scarlet & Violet print: https://api.pokemontcg.io/v2/cards/sv1-166
      .name = "Arven",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact Supporter subtype: https://api.pokemontcg.io/v2/cards/sv1-166
      .source_url = "https://api.pokemontcg.io/v2/cards/sv1-166", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3577
  };
};

}  // namespace sim::cards
