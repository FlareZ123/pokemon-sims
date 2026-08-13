#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class FieldBlower final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::FieldBlower,
      .canonical_id = "sm2-125", // Card data: https://api.pokemontcg.io/v2/cards/sm2-125
      .name = "Field Blower",
      .kind = CardKind::Trainer, // Rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L404
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/sm2-125", // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3512
  };
};

}  // namespace sim::cards
