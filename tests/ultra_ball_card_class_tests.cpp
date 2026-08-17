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

void test_registry_metadata_and_intrinsic_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::UltraBall);
  require(definition != nullptr, "Ultra Ball must be explicitly registered.");
  require(definition->id == sim::Card::UltraBall,
          "Ultra Ball registry id must remain stable.");
  require(definition->canonical_id == "sv1-196",
          "Ultra Ball canonical print changed.");  // Exact Scarlet & Violet record: https://api.pokemontcg.io/v2/cards?q=id:sv1-196
  require(definition->name == "Ultra Ball", "Ultra Ball name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Ultra Ball must remain a Trainer.");  // Exact card record: https://api.pokemontcg.io/v2/cards?q=id:sv1-196
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Ultra Ball must remain an Item.");  // Printed Item classification: https://api.pokemontcg.io/v2/cards?q=id:sv1-196 ; Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::cards::registered_is_item(sim::Card::UltraBall),
          "Registry classification must source Ultra Ball as an Item.");
  require(sim::is_item(sim::Card::UltraBall),
          "Compatibility classification must source Ultra Ball as an Item.");
  require(sim::name(sim::Card::UltraBall) == "Ultra Ball",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_intrinsic_classification();
    std::cout << "Ultra Ball card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
