#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_registry_metadata() {
  const auto* definition = sim::cards::find_definition(sim::Card::RegidragoV);
  require(definition != nullptr,
          "Regidrago V must be explicitly registered.");
  require(definition->canonical_id == "swsh12-135",
          "Regidrago V canonical print changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition->name == "Regidrago V",
          "Regidrago V display name changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition->kind == sim::cards::CardKind::Pokemon,
          "Regidrago V must remain a Pokémon."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition->pokemon_stage == sim::cards::PokemonStage::Basic,
          "Regidrago V must remain Basic."); // Exact subtype: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::cards::has_pokemon_type(*definition, sim::cards::PokemonType::Dragon),
          "Regidrago V must remain Dragon type."); // Exact type: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition->retreat_cost == 3,
          "Regidrago V metadata must preserve Retreat Cost 3."); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition->rule_box && definition->pokemon_v,
          "Regidrago V must retain Rule Box and Pokémon V metadata."); // Pokémon V print: https://api.pokemontcg.io/v2/cards/swsh12-135
}

void test_legacy_intrinsic_parity() {
  require(sim::name(sim::Card::RegidragoV) == "Regidrago V",
          "Registered display name must preserve legacy traces."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::is_basic(sim::Card::RegidragoV),
          "Registered Regidrago V must remain Basic."); // Exact subtype: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::is_pokemon(sim::Card::RegidragoV),
          "Registered Regidrago V must remain a Pokémon."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::is_pokemon_v(sim::Card::RegidragoV),
          "Registered Regidrago V must remain a Pokémon V."); // Pokémon V print: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::is_rule_box_pokemon(sim::Card::RegidragoV),
          "Registered Regidrago V must remain a Rule Box Pokémon."); // Pokémon V rule box: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::is_dragon_or_psychic(sim::Card::RegidragoV) &&
              sim::is_dragon(sim::Card::RegidragoV),
          "Registered Regidrago V must remain a Dragon search target."); // Exact type: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::retreat_cost(sim::Card::RegidragoV) == 3,
          "Cleanup must preserve the live Retreat Cost resolver."); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-135 ; confirmed sibling bug remains separate: https://github.com/FlareZ123/pokemon-sims/issues/3652
}

}  // namespace

int main() {
  try {
    test_registry_metadata();
    test_legacy_intrinsic_parity();
    std::cout << "Regidrago V card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
