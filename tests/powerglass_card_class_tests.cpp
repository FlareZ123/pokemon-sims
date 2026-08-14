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
  const auto* definition = sim::cards::find_definition(sim::Card::Powerglass);
  require(definition != nullptr,
          "Powerglass must be explicitly registered.");
  require(definition->id == sim::Card::Powerglass,
          "Powerglass registry id must remain stable.");
  require(definition->canonical_id == "sv6pt5-63",
          "Powerglass canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  require(definition->name == "Powerglass",
          "Powerglass name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Powerglass must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Tool,
          "Powerglass must remain a Pokémon Tool."); // Exact print: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  require(sim::is_tool(sim::Card::Powerglass),
          "Compatibility classification must source Powerglass as a Tool.");
  require(!sim::is_item(sim::Card::Powerglass),
          "Powerglass must not be classified as an Item.");
  require(sim::name(sim::Card::Powerglass) == "Powerglass",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_tool_classification();
    std::cout << "Powerglass card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
