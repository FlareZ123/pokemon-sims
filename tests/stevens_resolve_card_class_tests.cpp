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
  const auto* definition = sim::cards::find_definition(sim::Card::StevensResolve);
  require(definition != nullptr, "Steven's Resolve must be explicitly registered.");
  require(definition->id == sim::Card::StevensResolve,
          "Steven's Resolve registry id must remain stable.");
  require(definition->canonical_id == "sm7-145",
          "Steven's Resolve canonical print changed.");  // Exact card data: https://api.pokemontcg.io/v2/cards/sm7-145
  require(definition->name == "Steven's Resolve", "Steven's Resolve name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Steven's Resolve must remain a Trainer.");  // Trainer identity: https://api.pokemontcg.io/v2/cards/sm7-145
  require(definition->trainer_kind == sim::cards::TrainerKind::Supporter,
          "Steven's Resolve must remain a Supporter.");  // Printed subtype: https://api.pokemontcg.io/v2/cards/sm7-145 ; procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  require(sim::is_supporter(sim::Card::StevensResolve),
          "Compatibility classification must source Steven's Resolve as a Supporter.");
  require(!sim::is_item(sim::Card::StevensResolve),
          "Steven's Resolve must not be classified as an Item.");  // Printed Trainer subtype: https://api.pokemontcg.io/v2/cards/sm7-145
  require(sim::name(sim::Card::StevensResolve) == "Steven's Resolve",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_supporter_classification();
    std::cout << "Steven's Resolve card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
