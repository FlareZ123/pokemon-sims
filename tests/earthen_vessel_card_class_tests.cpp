#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_earthen_vessel_metadata_is_registry_owned() {
  const sim::cards::CardDefinition* definition =
      sim::cards::find_definition(sim::Card::EarthenVessel);

  // Earthen Vessel is an Item named "Earthen Vessel" in Paradox Rift.
  // Exact card data: https://api.pokemontcg.io/v2/cards/sv4-163
  // Class-migration cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3475
  expect(definition != nullptr, "Earthen Vessel must be registered.");
  expect(definition->id == sim::Card::EarthenVessel,
         "Earthen Vessel registry identity must match Card::EarthenVessel.");
  expect(std::string_view{definition->canonical_id} == "sv4-163",
         "Earthen Vessel canonical card id must be sv4-163.");
  expect(std::string_view{definition->name} == "Earthen Vessel",
         "Earthen Vessel canonical name must match printed card data.");
  expect(definition->kind == sim::cards::CardKind::Trainer,
         "Earthen Vessel must remain a Trainer.");
  expect(definition->trainer_kind == sim::cards::TrainerKind::Item,
         "Earthen Vessel must remain an Item.");
  expect(sim::cards::registered_is_item(sim::Card::EarthenVessel),
         "Earthen Vessel Item classification must resolve through the registry.");
}

}  // namespace

int main() {
  try {
    test_earthen_vessel_metadata_is_registry_owned();
    std::cout << "Earthen Vessel card-class tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
