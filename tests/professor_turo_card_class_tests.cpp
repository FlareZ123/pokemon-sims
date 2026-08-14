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
  const auto* definition = sim::cards::find_definition(sim::Card::ProfessorTuro);
  require(definition != nullptr,
          "Professor Turo's Scenario must be explicitly registered.");
  require(definition->id == sim::Card::ProfessorTuro,
          "Professor Turo registry id must remain stable.");
  require(definition->canonical_id == "sv4-171",
          "Professor Turo canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sv4-171
  require(definition->name == "Professor Turo's Scenario",
          "Professor Turo display name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Professor Turo must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Professor Turo must remain a Supporter."); // Exact Supporter data: https://api.pokemontcg.io/v2/cards/sv4-171
  require(sim::is_supporter(sim::Card::ProfessorTuro),
          "Compatibility classification must source Professor Turo as a Supporter.");
  require(sim::name(sim::Card::ProfessorTuro) == "Professor Turo's Scenario",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Professor Turo card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}