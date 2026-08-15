#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class TateLiza final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::TateLiza,
      .canonical_id = "sm7-148", // Exact Celestial Storm print: https://api.pokemontcg.io/v2/cards/sm7-148
      .name = "Tate & Liza",
      .kind = CardKind::Trainer, // Trainer identity: https://api.pokemontcg.io/v2/cards/sm7-148
      .trainer_kind = TrainerKind::Supporter, // Printed Supporter subtype and one-Supporter reminder: https://api.pokemontcg.io/v2/cards/sm7-148 ; procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .source_url = "https://api.pokemontcg.io/v2/cards/sm7-148", // Reclaimed migration: https://github.com/FlareZ123/pokemon-sims/issues/3562
  };
};

}  // namespace sim::cards
