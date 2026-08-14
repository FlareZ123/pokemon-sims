#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class ErikasInvitation final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::ErikasInvitation,
      .canonical_id = "sv3pt5-160",  // Exact Pokémon 151 print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
      .name = "Erika's Invitation",
      .kind = CardKind::Trainer,  // Trainer identity and printed effect: https://api.pokemontcg.io/v2/cards/sv3pt5-160
      .trainer_kind = TrainerKind::Supporter,  // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/sv3pt5-160 ; procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .source_url = "https://api.pokemontcg.io/v2/cards/sv3pt5-160",  // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/issues/3598
  };
};

}  // namespace sim::cards
