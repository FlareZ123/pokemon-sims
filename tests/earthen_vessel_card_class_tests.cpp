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

using sim::Card;

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

void test_registry_metadata_and_legacy_compatibility() {
  const auto* definition = sim::cards::find_definition(Card::EarthenVessel);
  require(definition != nullptr,
          "Earthen Vessel must be explicitly registered.");
  require(definition->id == Card::EarthenVessel,
          "Earthen Vessel registry id must remain stable.");
  require(definition->canonical_id == "sv4-163",
          "Earthen Vessel canonical print changed."); // Exact card: https://api.pokemontcg.io/v2/cards/sv4-163
  require(definition->name == "Earthen Vessel",
          "Earthen Vessel name changed."); // Exact card: https://api.pokemontcg.io/v2/cards/sv4-163
  require(definition->kind == sim::cards::CardKind::Trainer,
          "Earthen Vessel must remain a Trainer.");
  require(definition->trainer_kind == sim::cards::TrainerKind::Item,
          "Earthen Vessel must remain an Item."); // Printed Item classification: https://api.pokemontcg.io/v2/cards/sv4-163
  require(sim::is_item(Card::EarthenVessel),
          "Legacy compatibility must source Earthen Vessel Item classification from the registry.");
  require(sim::name(Card::EarthenVessel) == "Earthen Vessel",
          "Legacy compatibility must source the Earthen Vessel display name from the registry.");
}

}  // namespace

int main() {
  try {
    test_registry_metadata_and_legacy_compatibility();
    std::cout << "Earthen Vessel card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
