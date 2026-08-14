#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class TeamYellsCheer final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::TeamYellsCheer,
      .canonical_id = "swsh9-149",  // Exact card data: https://api.pokemontcg.io/v2/cards/swsh9-149
      .name = "Team Yell's Cheer",
      .kind = CardKind::Trainer,  // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter,  // Printed Supporter subtype and one-per-turn reminder: https://api.pokemontcg.io/v2/cards/swsh9-149
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh9-149",  // Cleanup issue: https://github.com/FlareZ123/pokemon-sims/issues/3620
  };
};

}  // namespace sim::cards
