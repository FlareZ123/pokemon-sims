#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ProfessorTuroScenario final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ProfessorTuro,
      .canonical_id = "sv4-171", // Exact Paradox Rift Supporter: https://api.pokemontcg.io/v2/cards/sv4-171
      .name = "Professor Turo's Scenario",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact card data: https://api.pokemontcg.io/v2/cards/sv4-171
      .source_url = "https://api.pokemontcg.io/v2/cards/sv4-171", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3632
  };
};

}  // namespace sim::cards