#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

// Exact Guardians Rising print: https://api.pokemontcg.io/v2/cards/sm2-55
// Cleanup owner: https://github.com/FlareZ123/pokemon-sims/issues/3712
// Preserve the simulator's established print-qualified display label so readable
// seeded traces remain byte-compatible: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_2310_turo_oricorio_trace_order_tests.cpp
class Oricorio final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::Oricorio,
      .canonical_id = "sm2-55",
      .name = "Oricorio GRI 55",
      .kind = CardKind::Pokemon,
      .pokemon_stage = PokemonStage::Basic,
      .pokemon_types = {PokemonType::Psychic, PokemonType::None},
      .pokemon_type_count = 1,
      .retreat_cost = 1, // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/sm2-55
      .rule_box = false, // Exact print has no Rule Box: https://api.pokemontcg.io/v2/cards/sm2-55
      .source_url = "https://api.pokemontcg.io/v2/cards/sm2-55",
  };
};

static_assert(Oricorio::definition.id == Card::Oricorio);
static_assert(Oricorio::definition.canonical_id == "sm2-55");
static_assert(Oricorio::definition.name == "Oricorio GRI 55"); // Compatibility trace contract: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_2310_turo_oricorio_trace_order_tests.cpp
static_assert(Oricorio::definition.pokemon_stage == PokemonStage::Basic);
static_assert(has_pokemon_type(Oricorio::definition, PokemonType::Psychic));
static_assert(Oricorio::definition.retreat_cost == 1); // Exact card data: https://api.pokemontcg.io/v2/cards/sm2-55
static_assert(!Oricorio::definition.rule_box); // Exact card data: https://api.pokemontcg.io/v2/cards/sm2-55

}  // namespace sim::cards
