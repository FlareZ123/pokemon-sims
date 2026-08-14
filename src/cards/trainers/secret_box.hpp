#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

class SecretBox final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::SecretBox,
      .canonical_id = "sv6-163",  // Exact card data: https://api.pokemontcg.io/v2/cards/sv6-163
      .name = "Secret Box",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,  // Printed Item classification: https://api.pokemontcg.io/v2/cards/sv6-163
      .ace_spec = true,  // Printed ACE SPEC classification: https://api.pokemontcg.io/v2/cards/sv6-163
      .source_url = "https://api.pokemontcg.io/v2/cards/sv6-163",
  };
};

}  // namespace sim::cards
