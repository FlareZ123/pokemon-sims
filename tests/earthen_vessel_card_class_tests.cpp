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
  const auto* definition = sim::cards::find_definition(sim::Card::EarthenVessel);
  require(definition != nullptr,
          "Earthen Vessel must be explicitly registered.");
  require(definition->id == sim::Card::EarthenVessel,
          "Earthen Vessel registry id must remain stable.");
  require(definition->canonical_id == "sv4-163",
          "Earthen Vessel canonical print changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sv4-163
  require(definition->name == "Earthen Vessel",
          "Earthen Vessel name changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sv4-163
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Earthen Vessel must remain a Trainer.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sv4-163
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Earthen Vessel must remain an Item.");  // Printed Item classification: https://api.pokemontcg.io/v2/cards/sv4-163 ; Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::cards::registered_is_item(sim::Card::EarthenVessel),
          "Registered classification must identify Earthen Vessel as an Item.");
  require(sim::is_item(sim::Card::EarthenVessel),
          "Compatibility classification must source Earthen Vessel as an Item.");
  require(sim::name(sim::Card::EarthenVessel) == "Earthen Vessel",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_intrinsic_classification();
    std::cout << "Earthen Vessel card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
