#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_wonder_tag_yields_known_dead_burnet_to_arven() {
  const sim::Scenario scenario{"wonder-tag-k1", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  // Engine stores the recipe by reference, so this fixture keeps it alive.
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{31415};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Arven,
                sim::Card::EvolutionIncense, sim::Card::ForestSealStone};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                  sim::Card::GoodraVstar, sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag searches the deck for a Supporter after Tapu Lele-GX is played
  // from hand to the Bench: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should resolve Wonder Tag from the Bench.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  // Professor Burnet searches the deck before discarding selected cards, so the
  // inspected payload-empty deck makes it dead: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Arven remains the live VSTAR connector: https://api.pokemontcg.io/v2/cards/sv1-166
  if (!contains(after.hand, sim::Card::Arven) || contains(after.hand, sim::Card::ProfessorBurnet)) {
    throw std::runtime_error("Wonder Tag should select Arven instead of a known-dead Professor Burnet.");
  }
}

void test_wonder_tag_uses_arven_vessel_for_final_energy() {
  const sim::Scenario scenario{"wonder-tag-arven-vessel-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{197};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Dipplin};
  state.deck = {sim::Card::Arven, sim::Card::EarthenVessel, sim::Card::Fire, sim::Card::Grass};
  // Keep the fixture focused on the Wonder Tag route. Otherwise Legacy Star may
  // legally recover paid costs after the chain and obscure the cost assertions.
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag may find Arven, Arven may find Earthen Vessel, and Vessel may
  // search the final Basic Fire Energy for the turn's manual attachment:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should resolve Wonder Tag for the Energy connector.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Arven)) {
    throw std::runtime_error("Wonder Tag should select Arven for the live Earthen Vessel route.");
  }

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->grass != 2 || after.active->fire != 1) {
    throw std::runtime_error("Arven into Earthen Vessel should complete GGF with the manual attachment.");
  }
  if (!after.supporter_used || !contains(after.discard, sim::Card::Arven) ||
      !contains(after.discard, sim::Card::EarthenVessel) || !contains(after.discard, sim::Card::Dipplin)) {
    throw std::runtime_error("The full Wonder Tag, Arven, and Earthen Vessel chain should resolve its costs.");
  }
}

}  // namespace

int main() {
  try {
    test_wonder_tag_yields_known_dead_burnet_to_arven();
    test_wonder_tag_uses_arven_vessel_for_final_energy();
    std::cout << "Wonder Tag known-dead Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
