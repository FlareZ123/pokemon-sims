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

void test_registry_metadata_and_item_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::EvolutionIncense);
  require(definition != nullptr,
          "Evolution Incense must be explicitly registered.");
  require(definition->id == sim::Card::EvolutionIncense,
          "Evolution Incense registry id must remain stable.");
  require(definition->canonical_id == "swsh1-163",
          "Evolution Incense canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/swsh1-163
  require(definition->name == "Evolution Incense",
          "Evolution Incense name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Evolution Incense must remain a Trainer."); // Rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Evolution Incense must remain an Item.");
  require(sim::is_item(sim::Card::EvolutionIncense),
          "Compatibility classification must source Evolution Incense as an Item.");
  require(sim::name(sim::Card::EvolutionIncense) == "Evolution Incense",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_item_classification();
    std::cout << "Evolution Incense card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
