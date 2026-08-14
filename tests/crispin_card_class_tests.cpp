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
  const auto* definition = sim::cards::find_definition(sim::Card::Crispin);
  require(definition != nullptr,
          "Crispin must be explicitly registered.");
  require(definition->id == sim::Card::Crispin,
          "Crispin registry id must remain stable.");
  require(definition->canonical_id == "sv7-133",
          "Crispin canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sv7-133
  require(definition->name == "Crispin",
          "Crispin name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Crispin must remain a Trainer."); // Exact Trainer identity: https://api.pokemontcg.io/v2/cards/sv7-133
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Crispin must remain a Supporter."); // Printed Supporter subtype: https://api.pokemontcg.io/v2/cards/sv7-133
  require(sim::is_supporter(sim::Card::Crispin),
          "Compatibility classification must source Crispin as a Supporter."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(!sim::is_item(sim::Card::Crispin),
          "Crispin must not be classified as an Item.");
  require(sim::name(sim::Card::Crispin) == "Crispin",
          "Compatibility name must source the registered definition.");
  // Printed Basic-Energy selection and attachment remain with Engine this wave.
  // Exact effect: https://api.pokemontcg.io/v2/cards/sv7-133 ; cleanup boundary: https://github.com/FlareZ123/pokemon-sims/issues/3580
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Crispin card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
