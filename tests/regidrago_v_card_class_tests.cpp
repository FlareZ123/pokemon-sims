#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include "../src/cards/pokemon/regidrago_v.hpp"

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

void test_exact_metadata() {
  const auto& definition = sim::cards::RegidragoV::definition;
  require(definition.id == sim::Card::RegidragoV,
          "Regidrago V simulator id must remain stable.");
  require(definition.canonical_id == "swsh12-135",
          "Regidrago V canonical print changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition.name == "Regidrago V",
          "Regidrago V display name changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition.kind == sim::cards::CardKind::Pokemon,
          "Regidrago V must remain a Pokémon."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition.pokemon_stage == sim::cards::PokemonStage::Basic,
          "Regidrago V must remain Basic."); // Exact subtype: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(sim::cards::has_pokemon_type(definition, sim::cards::PokemonType::Dragon),
          "Regidrago V must remain Dragon type."); // Exact type: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition.retreat_cost == 3,
          "Regidrago V metadata must preserve Retreat Cost 3."); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-135
  require(definition.rule_box && definition.pokemon_v,
          "Regidrago V must retain Rule Box and Pokémon V metadata."); // Pokémon V print: https://api.pokemontcg.io/v2/cards/swsh12-135
}

}  // namespace

int main() {
  try {
    test_exact_metadata();
    std::cout << "Regidrago V card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
