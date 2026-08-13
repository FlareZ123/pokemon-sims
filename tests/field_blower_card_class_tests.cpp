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

void test_registry_metadata_and_item_classification() {
  const auto* definition = sim::cards::find_definition(sim::Card::FieldBlower);
  require(definition != nullptr,
          "Field Blower must be explicitly registered.");
  require(definition->id == sim::Card::FieldBlower,
          "Field Blower registry id must remain stable.");
  require(definition->canonical_id == "sm2-125",
          "Field Blower canonical print changed."); // Card data: https://api.pokemontcg.io/v2/cards/sm2-125
  require(definition->name == "Field Blower",
          "Field Blower name changed.");
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Field Blower must remain a Trainer."); // Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L404
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Field Blower must remain an Item.");
  require(sim::is_item(sim::Card::FieldBlower),
          "Compatibility classification must source Field Blower as an Item.");
  require(sim::name(sim::Card::FieldBlower) == "Field Blower",
          "Compatibility name must source the registered definition.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_item_classification();
    std::cout << "Field Blower card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
