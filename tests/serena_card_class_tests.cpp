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
  const auto* definition = sim::cards::find_definition(sim::Card::Serena);
  require(definition != nullptr,
          "Serena must be explicitly registered.");
  require(definition->id == sim::Card::Serena,
          "Serena registry id must remain stable.");
  require(definition->canonical_id == "swsh12-164",
          "Serena canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/swsh12-164
  require(definition->name == "Serena",
          "Serena name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Serena must remain a Trainer."); // Exact Trainer identity: https://api.pokemontcg.io/v2/cards/swsh12-164
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Serena must remain a Supporter."); // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/swsh12-164
  require(sim::is_supporter(sim::Card::Serena),
          "Compatibility classification must source Serena as a Supporter."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(!sim::is_item(sim::Card::Serena),
          "Serena must not be classified as an Item.");
  require(sim::name(sim::Card::Serena) == "Serena",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Serena card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
