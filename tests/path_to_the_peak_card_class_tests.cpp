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

void test_registry_metadata_and_stadium_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::PathToPeak);
  require(definition != nullptr,
          "Path to the Peak must be explicitly registered.");
  require(definition->id == sim::Card::PathToPeak,
          "Path to the Peak registry id must remain stable.");
  require(definition->canonical_id == "swsh6-148",
          "Path to the Peak canonical print changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh6-148
  require(definition->name == "Path to the Peak",
          "Path to the Peak name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Path to the Peak must remain a Trainer."); // Stadium procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Stadium,
          "Path to the Peak must remain a Stadium."); // Printed Stadium classification: https://api.pokemontcg.io/v2/cards/swsh6-148
  require(sim::is_stadium(sim::Card::PathToPeak),
          "Compatibility classification must source Path to the Peak as a Stadium.");
  require(sim::name(sim::Card::PathToPeak) == "Path to the Peak",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_stadium_classification();
    std::cout << "Path to the Peak card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
