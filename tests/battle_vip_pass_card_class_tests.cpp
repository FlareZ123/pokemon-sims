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

void test_registry_metadata_and_item_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::BattleVipPass);
  require(definition != nullptr, "Battle VIP Pass must be explicitly registered.");
  require(definition->id == sim::Card::BattleVipPass, "Battle VIP Pass registry id must remain stable.");
  require(definition->canonical_id == "swsh8-225", "Battle VIP Pass canonical print changed."); // https://api.pokemontcg.io/v2/cards/swsh8-225
  require(definition->name == "Battle VIP Pass", "Battle VIP Pass name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer, "Battle VIP Pass must remain a Trainer."); // https://api.pokemontcg.io/v2/cards/swsh8-225
  require(definition->trainer_kind == sim::cards::TrainerKind::Item, "Battle VIP Pass must remain an Item.");
  require(sim::is_item(sim::Card::BattleVipPass), "Compatibility classification must source Battle VIP Pass as an Item.");
  require(sim::name(sim::Card::BattleVipPass) == "Battle VIP Pass", "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_item_classification();
    std::cout << "Battle VIP Pass card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
