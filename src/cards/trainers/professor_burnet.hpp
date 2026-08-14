#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ProfessorBurnet final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ProfessorBurnet,
      .canonical_id = "swsh12tg-TG26", // Exact Trainer Gallery print: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      .name = "Professor Burnet",
      .kind = CardKind::Trainer, // Trainer/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#b-03-supporters
      .trainer_kind = TrainerKind::Supporter,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12tg-TG26", // Cleanup enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3569
  };
};

}  // namespace sim::cards
