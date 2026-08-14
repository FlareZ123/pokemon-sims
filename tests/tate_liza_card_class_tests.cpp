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
  const auto* definition = sim::cards::find_definition(sim::Card::TateLiza);
  require(definition != nullptr,
          "Tate & Liza must be explicitly registered.");
  require(definition->id == sim::Card::TateLiza,
          "Tate & Liza registry id must remain stable.");
  require(definition->canonical_id == "sm7-148",
          "Tate & Liza canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sm7-148
  require(definition->name == "Tate & Liza",
          "Tate & Liza name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Tate & Liza must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Tate & Liza must remain a Supporter."); // Exact print: https://api.pokemontcg.io/v2/cards/sm7-148
  require(sim::is_supporter(sim::Card::TateLiza),
          "Compatibility classification must source Tate & Liza as a Supporter.");
  require(!sim::is_item(sim::Card::TateLiza),
          "Tate & Liza must not be classified as an Item.");
  require(sim::name(sim::Card::TateLiza) == "Tate & Liza",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Tate & Liza card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
