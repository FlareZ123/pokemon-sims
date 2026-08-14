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
  const auto* definition = sim::cards::find_definition(sim::Card::WishfulBaton);
  require(definition != nullptr,
          "Wishful Baton must be explicitly registered.");
  require(definition->id == sim::Card::WishfulBaton,
          "Wishful Baton registry id must remain stable.");
  require(definition->canonical_id == "sm3-128",
          "Wishful Baton canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sm3-128
  require(definition->name == "Wishful Baton",
          "Wishful Baton name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Wishful Baton must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Tool,
          "Wishful Baton must remain a Pokémon Tool."); // Exact print: https://api.pokemontcg.io/v2/cards/sm3-128
  require(sim::is_tool(sim::Card::WishfulBaton),
          "Compatibility classification must source Wishful Baton as a Tool.");
  require(!sim::is_item(sim::Card::WishfulBaton),
          "Wishful Baton must remain outside the Item classifier."); // Repository Trainer-kind architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_definition.hpp
  require(sim::name(sim::Card::WishfulBaton) == "Wishful Baton",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_tool_classification();
    std::cout << "Wishful Baton card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
