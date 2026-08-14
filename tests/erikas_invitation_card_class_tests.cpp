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
  const auto* definition = sim::cards::find_definition(sim::Card::ErikasInvitation);
  require(definition != nullptr,
          "Erika's Invitation must be explicitly registered.");
  require(definition->id == sim::Card::ErikasInvitation,
          "Erika's Invitation registry id must remain stable.");
  require(definition->canonical_id == "sv3pt5-160",
          "Erika's Invitation canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sv3pt5-160
  require(definition->name == "Erika's Invitation",
          "Erika's Invitation name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Erika's Invitation must remain a Trainer."); // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Erika's Invitation must remain a Supporter."); // Exact print: https://github.com/PokemonTCG/pokemon-tcg-data/blob/master/cards/en/sv3pt5.json
  require(sim::is_supporter(sim::Card::ErikasInvitation),
          "Compatibility classification must source Erika's Invitation as a Supporter.");
  require(!sim::is_item(sim::Card::ErikasInvitation),
          "Erika's Invitation must not be classified as an Item.");
  require(sim::name(sim::Card::ErikasInvitation) == "Erika's Invitation",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Erika's Invitation card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
