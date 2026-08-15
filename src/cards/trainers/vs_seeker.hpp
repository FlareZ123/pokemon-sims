#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class VsSeeker final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::VsSeeker,
      .canonical_id = "xy4-109", // Exact Phantom Forces print: https://api.pokemontcg.io/v2/cards/xy4-109
      .name = "VS Seeker",
      .kind = CardKind::Trainer, // Printed Trainer identity: https://api.pokemontcg.io/v2/cards/xy4-109
      .trainer_kind = TrainerKind::Item, // Printed Item subtype; Item procedure: https://api.pokemontcg.io/v2/cards/xy4-109 ; https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      .source_url = "https://api.pokemontcg.io/v2/cards/xy4-109", // Exact card data: https://api.pokemontcg.io/v2/cards/xy4-109
  };
};

}  // namespace sim::cards
