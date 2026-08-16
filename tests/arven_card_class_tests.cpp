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

void test_registry_metadata_and_supporter_classification() {
  const auto& definition = test_support::require_card_definition(
      sim::Card::Arven, "Arven must be explicitly registered.");
  test_support::require(definition.id == sim::Card::Arven,
                        "Arven registry id must remain stable.");
  test_support::require(definition.canonical_id == "sv1-166",
                        "Arven canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sv1-166
  test_support::require(definition.name == "Arven",
                        "Arven name changed.");
  test_support::require(definition.kind == sim::cards::CardKind::Trainer,
                        "Arven must remain a Trainer."); // Exact Trainer identity: https://api.pokemontcg.io/v2/cards/sv1-166
  test_support::require(definition.trainer_kind == sim::cards::TrainerKind::Supporter,
                        "Arven must remain a Supporter."); // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/sv1-166
  test_support::require(
      sim::is_supporter(sim::Card::Arven),
      "Compatibility classification must source Arven as a Supporter."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  test_support::require(!sim::is_item(sim::Card::Arven),
                        "Arven must not be classified as an Item.");
  test_support::require(sim::name(sim::Card::Arven) == "Arven",
                        "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Arven card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
