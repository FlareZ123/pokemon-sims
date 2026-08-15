#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class BattleCompressor final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::BattleCompressor,
      .canonical_id = "xy4-92", // Exact Phantom Forces print: https://api.pokemontcg.io/v2/cards/xy4-92
      .name = "Battle Compressor Team Flare Gear",
      .kind = CardKind::Trainer, // Printed Trainer identity: https://api.pokemontcg.io/v2/cards/xy4-92
      .trainer_kind = TrainerKind::Item, // Printed Item subtype; Item procedure: https://api.pokemontcg.io/v2/cards/xy4-92 ; https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .source_url = "https://api.pokemontcg.io/v2/cards/xy4-92", // Exact card data: https://api.pokemontcg.io/v2/cards/xy4-92
  };
};

}  // namespace sim::cards
