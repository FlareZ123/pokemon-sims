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
  const auto* definition = sim::cards::find_definition(sim::Card::Grant);
  require(definition != nullptr, "Grant must be explicitly registered.");
  require(definition->id == sim::Card::Grant, "Grant registry id must remain stable.");
  require(definition->canonical_id == "swsh10-144",
          "Grant canonical print changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/swsh10-144
  require(definition->name == "Grant", "Grant name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Grant must remain a Trainer.");  // Trainer identity: https://api.pokemontcg.io/v2/cards/swsh10-144
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Grant must remain a Supporter.");  // Printed subtype: https://api.pokemontcg.io/v2/cards/swsh10-144 ; procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::is_supporter(sim::Card::Grant),
          "Compatibility classification must source Grant as a Supporter.");
  require(!sim::is_item(sim::Card::Grant),
          "Grant must not be classified as an Item.");  // Printed Trainer subtype: https://api.pokemontcg.io/v2/cards/swsh10-144
  require(sim::name(sim::Card::Grant) == "Grant",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Grant card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
