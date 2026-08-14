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
  const auto* definition =
      sim::cards::find_definition(sim::Card::ProfessorBurnet);
  require(definition != nullptr,
          "Professor Burnet must be explicitly registered.");
  require(definition->id == sim::Card::ProfessorBurnet,
          "Professor Burnet registry id must remain stable.");
  require(definition->canonical_id == "swsh12tg-TG26",
          "Professor Burnet canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  require(definition->name == "Professor Burnet",
          "Professor Burnet name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Professor Burnet must remain a Trainer."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Professor Burnet must remain a Supporter."); // Card data: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  require(sim::is_supporter(sim::Card::ProfessorBurnet),
          "Compatibility classification must source Professor Burnet as a Supporter.");
  require(!sim::is_item(sim::Card::ProfessorBurnet),
          "Professor Burnet must not be classified as an Item.");
  require(sim::name(sim::Card::ProfessorBurnet) == "Professor Burnet",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Professor Burnet card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
