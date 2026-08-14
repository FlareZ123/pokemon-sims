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
  const auto* definition = sim::cards::find_definition(sim::Card::TeamYellsCheer);
  require(definition != nullptr,
          "Team Yell's Cheer must be explicitly registered.");
  require(definition->id == sim::Card::TeamYellsCheer,
          "Team Yell's Cheer registry id must remain stable.");
  require(definition->canonical_id == "swsh9-149",
          "Team Yell's Cheer canonical print changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/swsh9-149
  require(definition->name == "Team Yell's Cheer",
          "Team Yell's Cheer name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Team Yell's Cheer must remain a Trainer.");  // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Team Yell's Cheer must remain a Supporter.");  // Printed subtype: https://api.pokemontcg.io/v2/cards/swsh9-149
  require(sim::is_supporter(sim::Card::TeamYellsCheer),
          "Compatibility classification must source Team Yell's Cheer as a Supporter.");
  require(!sim::is_item(sim::Card::TeamYellsCheer),
          "Team Yell's Cheer must not be classified as an Item.");
  require(sim::name(sim::Card::TeamYellsCheer) == "Team Yell's Cheer",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Team Yell's Cheer card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
