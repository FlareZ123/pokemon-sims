#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class PokemonCommunication final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::PokemonCommunication,
      .canonical_id = "sm9-152",  // Exact card data: https://api.pokemontcg.io/v2/cards/sm9-152
      .name = "Pokémon Communication",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,  // Printed Item classification: https://api.pokemontcg.io/v2/cards/sm9-152
      .source_url = "https://api.pokemontcg.io/v2/cards/sm9-152",
  };
};

}  // namespace sim::cards
