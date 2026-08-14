#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_registry_metadata_and_supporter_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::RoseannesBackup);
  require(definition != nullptr, "Roseanne's Backup must be explicitly registered.");
  require(definition->canonical_id == "swsh9-148", "Roseanne's Backup canonical print changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/swsh9-148
  require(definition->name == "Roseanne's Backup", "Roseanne's Backup name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer, "Roseanne's Backup must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter, "Roseanne's Backup must remain a Supporter."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::is_supporter(sim::Card::RoseannesBackup), "Compatibility classification must use the registered Supporter subtype.");
  require(sim::name(sim::Card::RoseannesBackup) == "Roseanne's Backup", "Compatibility name must use the registered definition.");
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
