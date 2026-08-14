#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_registry_metadata_and_intrinsic_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::PokemonCommunication);
  require(definition != nullptr,
          "Pokémon Communication must be explicitly registered.");
  require(definition->id == sim::Card::PokemonCommunication,
          "Pokémon Communication registry id must remain stable.");
  require(definition->canonical_id == "sm9-152",
          "Pokémon Communication canonical print changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sm9-152
  require(definition->name == "Pokémon Communication",
          "Pokémon Communication name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Pokémon Communication must remain a Trainer.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sm9-152
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Pokémon Communication must remain an Item.");  // Printed Item classification: https://api.pokemontcg.io/v2/cards/sm9-152
  require(sim::is_item(sim::Card::PokemonCommunication),
          "Compatibility classification must source Pokémon Communication as an Item.");
  require(sim::name(sim::Card::PokemonCommunication) == "Pokémon Communication",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_intrinsic_classification();
    std::cout << "Pokémon Communication card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
