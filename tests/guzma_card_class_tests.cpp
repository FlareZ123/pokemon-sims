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

void test_registry_metadata_and_supporter_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::Guzma);
  require(definition != nullptr,
          "Guzma must be explicitly registered.");
  require(definition->id == sim::Card::Guzma,
          "Guzma registry id must remain stable.");
  require(definition->canonical_id == "sm3-115",
          "Guzma canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sm3-115
  require(definition->name == "Guzma",
          "Guzma name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Guzma must remain a Trainer."); // Exact Trainer identity: https://api.pokemontcg.io/v2/cards/sm3-115
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Guzma must remain a Supporter."); // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/sm3-115
  require(sim::is_supporter(sim::Card::Guzma),
          "Compatibility classification must source Guzma as a Supporter."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(!sim::is_item(sim::Card::Guzma),
          "Guzma must not be classified as an Item.");
  require(sim::name(sim::Card::Guzma) == "Guzma",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Guzma card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
