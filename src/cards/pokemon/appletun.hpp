#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

// Exact Surging Sparks print: https://api.pokemontcg.io/v2/cards/sv8-140
// Cleanup owner: https://github.com/FlareZ123/pokemon-sims/issues/3642
// The simulator's established display label includes the canonical print id so
// seeded traces stay byte-for-byte compatible: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1839_dawn_secret_box_refill_tests.cpp
// Live retreat behavior is owned separately: https://github.com/FlareZ123/pokemon-sims/issues/3643
class Appletun final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Appletun,
      .canonical_id = "sv8-140",
      .name = "Appletun sv8-140",
      .kind = CardKind::Pokemon,
      .pokemon_stage = PokemonStage::Stage1,
      .pokemon_types = {PokemonType::Dragon, PokemonType::None},
      .pokemon_type_count = 1,
      .retreat_cost = 3,
      .source_url = "https://api.pokemontcg.io/v2/cards/sv8-140",
  };
};

static_assert(Appletun::definition.id == Card::Appletun);
static_assert(Appletun::definition.canonical_id == "sv8-140");
static_assert(Appletun::definition.name == "Appletun sv8-140"); // Compatibility trace contract: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1118_multi_deck_secret_box_tests.cpp
static_assert(Appletun::definition.pokemon_stage == PokemonStage::Stage1);
static_assert(has_pokemon_type(Appletun::definition, PokemonType::Dragon));
static_assert(Appletun::definition.retreat_cost == 3); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/sv8-140

}  // namespace sim::cards
