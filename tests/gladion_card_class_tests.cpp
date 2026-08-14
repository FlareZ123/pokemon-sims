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
  const auto* definition = sim::cards::find_definition(sim::Card::Gladion);
  require(definition != nullptr, "Gladion must be explicitly registered.");
  require(definition->id == sim::Card::Gladion,
          "Gladion registry id must remain stable.");
  require(definition->canonical_id == "sm4-95",
          "Gladion canonical print changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sm4-95
  require(definition->name == "Gladion", "Gladion name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Gladion must remain a Trainer.");  // Trainer identity: https://api.pokemontcg.io/v2/cards/sm4-95
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Gladion must remain a Supporter.");  // Printed subtype: https://api.pokemontcg.io/v2/cards/sm4-95 ; procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::is_supporter(sim::Card::Gladion),
          "Compatibility classification must source Gladion as a Supporter.");
  require(!sim::is_item(sim::Card::Gladion),
          "Gladion must not be classified as an Item.");  // Printed Trainer subtype: https://api.pokemontcg.io/v2/cards/sm4-95
  require(sim::name(sim::Card::Gladion) == "Gladion",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Gladion card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
