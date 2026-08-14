#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

// Exact Unified Minds print: https://api.pokemontcg.io/v2/cards/sm11-141
// Cleanup owner: https://github.com/FlareZ123/pokemon-sims/issues/3704
class MawileGX final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::MawileGX,
      .canonical_id = "sm11-141",
      .name = "Mawile-GX",
      .kind = CardKind::Pokemon,
      .pokemon_stage = PokemonStage::Basic,
      .pokemon_types = {PokemonType::Metal, PokemonType::None},
      .pokemon_type_count = 1,
      .retreat_cost = 1, // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/sm11-141
      .rule_box = true, // Pokémon-GX Rule Box identity: https://api.pokemontcg.io/v2/cards/sm11-141
      .source_url = "https://api.pokemontcg.io/v2/cards/sm11-141",
  };
};

static_assert(MawileGX::definition.id == Card::MawileGX);
static_assert(MawileGX::definition.canonical_id == "sm11-141");
static_assert(MawileGX::definition.name == "Mawile-GX");
static_assert(MawileGX::definition.pokemon_stage == PokemonStage::Basic);
static_assert(has_pokemon_type(MawileGX::definition, PokemonType::Metal));
static_assert(MawileGX::definition.retreat_cost == 1); // Exact card data: https://api.pokemontcg.io/v2/cards/sm11-141
static_assert(MawileGX::definition.rule_box); // Pokémon-GX exact print: https://api.pokemontcg.io/v2/cards/sm11-141

}  // namespace sim::cards
