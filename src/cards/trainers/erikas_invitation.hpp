#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ErikasInvitation final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ErikasInvitation,
      .canonical_id = "sv3pt5-160", // Exact card data: https://api.pokemontcg.io/v2/cards/sv3pt5-160
      .name = "Erika's Invitation",
      .kind = CardKind::Trainer, // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .trainer_kind = TrainerKind::Supporter, // Exact subtype and printed effect: https://github.com/PokemonTCG/pokemon-tcg-data/blob/master/cards/en/sv3pt5.json
      .source_url = "https://api.pokemontcg.io/v2/cards/sv3pt5-160", // Cleanup issue: https://github.com/FlareZ123/pokemon-sims/issues/3598
  };
};

}  // namespace sim::cards
