#pragma once

#include "../card_definition.hpp"

namespace sim::cards {

// Exact Silver Tempest print: https://api.pokemontcg.io/v2/cards/swsh12-135
// Cleanup owner: https://github.com/FlareZ123/pokemon-sims/issues/3683
class RegidragoV final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::RegidragoV,
      .canonical_id = "swsh12-135",
      .name = "Regidrago V",
      .kind = CardKind::Pokemon,
      .pokemon_stage = PokemonStage::Basic,
      .pokemon_types = {PokemonType::Dragon, PokemonType::None},
      .pokemon_type_count = 1,
      .retreat_cost = 3,
      .rule_box = true,
      .pokemon_v = true,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12-135",
  };
};

// Exact Silver Tempest print: https://api.pokemontcg.io/v2/cards/swsh12-136
class RegidragoVstar final {
 public:
  static constexpr CardDefinition definition{
      .id = Card::RegidragoVstar,
      .canonical_id = "swsh12-136",
      .name = "Regidrago VSTAR",
      .kind = CardKind::Pokemon,
      .pokemon_stage = PokemonStage::VStar,
      .pokemon_types = {PokemonType::Dragon, PokemonType::None},
      .pokemon_type_count = 1,
      .retreat_cost = 3,
      .rule_box = true,
      .pokemon_v = true,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh12-136",
  };
};

static_assert(RegidragoV::definition.id == Card::RegidragoV);
static_assert(RegidragoV::definition.canonical_id == "swsh12-135");
static_assert(RegidragoV::definition.name == "Regidrago V");
static_assert(RegidragoV::definition.pokemon_stage == PokemonStage::Basic); // Exact subtype: https://api.pokemontcg.io/v2/cards/swsh12-135
static_assert(has_pokemon_type(RegidragoV::definition, PokemonType::Dragon)); // Exact type: https://api.pokemontcg.io/v2/cards/swsh12-135
static_assert(RegidragoV::definition.retreat_cost == 3); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-135
static_assert(RegidragoV::definition.rule_box);
static_assert(RegidragoV::definition.pokemon_v);

static_assert(RegidragoVstar::definition.id == Card::RegidragoVstar);
static_assert(RegidragoVstar::definition.canonical_id == "swsh12-136");
static_assert(RegidragoVstar::definition.name == "Regidrago VSTAR");
static_assert(RegidragoVstar::definition.pokemon_stage == PokemonStage::VStar); // Exact subtype: https://api.pokemontcg.io/v2/cards/swsh12-136
static_assert(has_pokemon_type(RegidragoVstar::definition, PokemonType::Dragon)); // Exact type: https://api.pokemontcg.io/v2/cards/swsh12-136
static_assert(RegidragoVstar::definition.retreat_cost == 3); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-136
static_assert(RegidragoVstar::definition.rule_box); // VSTAR rule: https://api.pokemontcg.io/v2/cards/swsh12-136
static_assert(RegidragoVstar::definition.pokemon_v); // Pokémon VSTAR is a Pokémon V: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/

}  // namespace sim::cards
