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

void test_ultra_ball_registry_metadata() {
  const auto* definition = sim::cards::find_definition(sim::Card::UltraBall);
  require(definition != nullptr, "Ultra Ball must be explicitly registered.");
  require(definition->canonical_id == "sv1-196",
          "Ultra Ball canonical print changed."); // Exact print: https://api.pokemontcg.io/v2/cards/sv1-196
  require(definition->name == "Ultra Ball", "Ultra Ball name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Ultra Ball must remain a Trainer.");
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Ultra Ball must remain an Item."); // Printed Item subtype: https://api.pokemontcg.io/v2/cards/sv1-196
  require(sim::cards::UltraBall::kDiscardCost == 2,
          "Ultra Ball must retain its two-other-card printed cost."); // Printed discard cost: https://api.pokemontcg.io/v2/cards/sv1-196
  require(sim::is_item(sim::Card::UltraBall),
          "Engine compatibility classification must use Ultra Ball Item metadata.");
  require(sim::name(sim::Card::UltraBall) == "Ultra Ball",
          "Engine compatibility name must use the registered definition.");
}

}  // namespace

int main() {
  try {
    test_ultra_ball_registry_metadata();
    std::cout << "Ultra Ball card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
