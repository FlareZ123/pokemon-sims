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
  const auto* definition = sim::cards::find_definition(sim::Card::ForestOfVitality);
  require(definition != nullptr,
          "Forest of Vitality must be explicitly registered.");
  require(definition->id == sim::Card::ForestOfVitality,
          "Forest of Vitality registry id must remain stable.");
  require(definition->canonical_id == "me1-117",
          "Forest of Vitality canonical print changed."); // Exact card data: https://api.pokemontcg.io/v2/cards/me1-117
  require(definition->name == "Forest of Vitality",
          "Forest of Vitality name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Forest of Vitality must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Stadium,
          "Forest of Vitality must remain a Stadium."); // Exact card data: https://api.pokemontcg.io/v2/cards/me1-117
  require(sim::is_stadium(sim::Card::ForestOfVitality),
          "Compatibility classification must source Forest of Vitality as a Stadium."); // Stadium classification: https://api.pokemontcg.io/v2/cards/me1-117
  require(!sim::is_item(sim::Card::ForestOfVitality),
          "Forest of Vitality must not be classified as an Item.");
  require(sim::name(sim::Card::ForestOfVitality) == "Forest of Vitality",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_stadium_classification();
    std::cout << "Forest of Vitality card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
