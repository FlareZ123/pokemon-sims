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

void test_registry_metadata_and_tool_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::ForestSealStone);
  require(definition != nullptr,
          "Forest Seal Stone must be explicitly registered.");
  require(definition->id == sim::Card::ForestSealStone,
          "Forest Seal Stone registry id must remain stable.");
  require(definition->canonical_id == "swsh12-156",
          "Forest Seal Stone canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/swsh12-156
  require(definition->name == "Forest Seal Stone",
          "Forest Seal Stone name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Forest Seal Stone must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Tool,
          "Forest Seal Stone must remain a Pokémon Tool."); // Exact print: https://api.pokemontcg.io/v2/cards/swsh12-156
  require(sim::is_tool(sim::Card::ForestSealStone),
          "Compatibility classification must source Forest Seal Stone as a Tool.");
  require(!sim::is_item(sim::Card::ForestSealStone),
          "Forest Seal Stone must not be classified as an Item."); // Tool procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::name(sim::Card::ForestSealStone) == "Forest Seal Stone",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_tool_classification();
    std::cout << "Forest Seal Stone card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
