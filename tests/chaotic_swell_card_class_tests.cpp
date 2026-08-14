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
  const auto* definition = sim::cards::find_definition(sim::Card::ChaoticSwell);
  require(definition != nullptr,
          "Chaotic Swell must be explicitly registered.");
  require(definition->id == sim::Card::ChaoticSwell,
          "Chaotic Swell registry id must remain stable.");
  require(definition->canonical_id == "sm12-187",
          "Chaotic Swell canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sm12-187
  require(definition->name == "Chaotic Swell",
          "Chaotic Swell name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Chaotic Swell must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Stadium,
          "Chaotic Swell must remain a Stadium."); // Exact print: https://api.pokemontcg.io/v2/cards/sm12-187
  require(sim::is_stadium(sim::Card::ChaoticSwell),
          "Compatibility classification must source Chaotic Swell as a Stadium."); // Exact print: https://api.pokemontcg.io/v2/cards/sm12-187
  require(!sim::is_item(sim::Card::ChaoticSwell),
          "Chaotic Swell must not be classified as an Item.");
  require(sim::name(sim::Card::ChaoticSwell) == "Chaotic Swell",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_stadium_classification();
    std::cout << "Chaotic Swell card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
