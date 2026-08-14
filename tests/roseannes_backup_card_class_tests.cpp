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
  const auto* definition = sim::cards::find_definition(sim::Card::RoseannesBackup);
  require(definition != nullptr,
          "Roseanne's Backup must be explicitly registered.");
  require(definition->id == sim::Card::RoseannesBackup,
          "Roseanne's Backup registry id must remain stable.");
  require(definition->canonical_id == "swsh9-148",
          "Roseanne's Backup canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/swsh9-148
  require(definition->name == "Roseanne's Backup",
          "Roseanne's Backup name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Roseanne's Backup must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Roseanne's Backup must remain a Supporter."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::is_supporter(sim::Card::RoseannesBackup),
          "Compatibility classification must source Roseanne's Backup as a Supporter.");
  require(sim::name(sim::Card::RoseannesBackup) == "Roseanne's Backup",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Roseanne's Backup card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
