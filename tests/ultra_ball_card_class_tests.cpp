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
  require(definition->id == sim::Card::UltraBall,
          "Ultra Ball registry id must remain stable.");
  require(definition->canonical_id == "sv1-196",
          "Ultra Ball canonical print changed.");
  require(definition->name == "Ultra Ball", "Ultra Ball name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Ultra Ball must remain a Trainer.");
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Ultra Ball must remain an Item.");
  require(sim::cards::UltraBall::kDiscardCost == 2,
          "Ultra Ball must preserve its printed two-other-card play cost.");
  require(sim::is_item(sim::Card::UltraBall),
          "Legacy classification must source Ultra Ball as an Item.");
  require(sim::name(sim::Card::UltraBall) == "Ultra Ball",
          "Legacy name must source the registered Ultra Ball definition.");
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
