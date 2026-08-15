#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class TateLiza final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::TateLiza,
      .canonical_id = "sm7-148", // Exact Celestial Storm print: https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/sm7.json
      .name = "Tate & Liza",
      .kind = CardKind::Trainer, // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact Supporter subtype: https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/sm7.json
      .source_url = "https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/sm7.json", // Migration owner: https://github.com/FlareZ123/pokemon-sims/issues/3562
  };
};

}  // namespace sim::cards
