#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include "support/card_registry_test_utils.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {

// Unified-test discovery requires one access block per standalone regression.
// This metadata seam is public, so no privileged Engine access is necessary.
struct EngineTestAccess {};

}  // namespace sim

namespace {

void test_registry_metadata() {
  const auto& definition = test_support::require_card_definition(
      sim::Card::Appletun, "Appletun must be explicitly registered.");
  test_support::require(definition.id == sim::Card::Appletun,
                        "Appletun registry id must remain stable.");
  test_support::require(definition.canonical_id == "sv8-140",
                        "Appletun canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sv8-140
  test_support::require(definition.name == "Appletun sv8-140",
                        "Appletun simulator display label changed."); // Trace compatibility: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1839_dawn_secret_box_refill_tests.cpp ; exact print: https://api.pokemontcg.io/v2/cards/sv8-140
  test_support::require(definition.kind == sim::cards::CardKind::Pokemon,
                        "Appletun must remain a Pokémon."); // Exact card data: https://api.pokemontcg.io/v2/cards/sv8-140
  test_support::require(definition.pokemon_stage == sim::cards::PokemonStage::Stage1,
                        "Appletun must remain Stage 1."); // Exact card data: https://api.pokemontcg.io/v2/cards/sv8-140
  test_support::require(
      sim::cards::has_pokemon_type(definition, sim::cards::PokemonType::Dragon),
      "Appletun must remain Dragon type."); // Exact card data: https://api.pokemontcg.io/v2/cards/sv8-140
  test_support::require(definition.retreat_cost == 3,
                        "Appletun metadata must preserve its printed Retreat Cost."); // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/sv8-140 ; live resolver owned separately: https://github.com/FlareZ123/pokemon-sims/issues/3643
}

}  // namespace

int main() {
  try {
    test_registry_metadata();
    std::cout << "Appletun card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
