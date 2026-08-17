#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class UltraBall final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::UltraBall,
      .canonical_id = "sv1-196",  // Exact Scarlet & Violet record: https://api.pokemontcg.io/v2/cards?q=id:sv1-196 ; confirmed in the supplied pokemon-tcg-data snapshot.
      .name = "Ultra Ball",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,  // Printed Item classification: https://api.pokemontcg.io/v2/cards?q=id:sv1-196 ; Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .source_url = "https://api.pokemontcg.io/v2/cards?q=id:sv1-196",
  };
};

}  // namespace sim::cards
