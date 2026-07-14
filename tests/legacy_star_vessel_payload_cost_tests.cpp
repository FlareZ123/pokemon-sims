#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_earthen_vessel_with_payload(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

std::vector<sim::Card> vessel_bridge_deck() {
  // Legacy Star removes cards from the vector back. Fire and Dragapult remain below
  // the seven-card discard, while Vessel and six strict-protected Trainers are hit.
  return {sim::Card::Fire, sim::Card::Dragapult, sim::Card::ForestSealStone,
          sim::Card::Powerglass, sim::Card::RoseannesBackup, sim::Card::Lusamine,
          sim::Card::ProfessorTuro, sim::Card::TeamYellsCheer,
          sim::Card::EarthenVessel};
}

void test_legacy_star_recovers_vessel_for_held_payload_cost() {
  using namespace sim;
  const Scenario scenario{"legacy-star-held-payload-vessel", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4171};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::MegaDragonite};
  state.deck = vessel_bridge_deck();

  // Legacy Star may recover Earthen Vessel. Vessel may discard the held Dragon,
  // which both pays its cost and establishes the same-turn Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  if (!EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star should recover Vessel when the held payload is its live cost.");
  }

  if (!EngineTestAccess::play_earthen_vessel_with_payload(engine) ||
      !contains(state.discard, Card::MegaDragonite) ||
      !contains(state.discarded_this_turn, Card::MegaDragonite) ||
      !contains(state.hand, Card::Fire)) {
    throw std::runtime_error("Vessel must discard the held payload and search the missing Fire Energy.");
  }

  // A player may attach one Energy during the turn. The recovered Fire completes GGF:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!EngineTestAccess::attach_manual(engine) || !state.active ||
      state.active->grass != 2 || state.active->fire != 1 ||
      !EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The Vessel bridge must complete both Energy and strict-JIT payload axes.");
  }
}

void test_legacy_star_rejects_payload_cost_before_window_opens() {
  using namespace sim;
  const Scenario scenario{"legacy-star-closed-payload-window", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4172};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::MegaDragonite};
  state.deck = vessel_bridge_deck();

  // Repository strict-JIT timing opens the payload window from turn 2. Before that
  // window, the held Dragon remains protected and cannot make Vessel payable:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  if (!EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The constructed control should still resolve Legacy Star.");
  }
  if (contains(state.hand, Card::EarthenVessel) ||
      !contains(state.hand, Card::MegaDragonite) ||
      EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("A closed payload window must preserve the Dragon and reject the Vessel bridge.");
  }
}

}  // namespace

int main() {
  try {
    test_legacy_star_recovers_vessel_for_held_payload_cost();
    test_legacy_star_rejects_payload_cost_before_window_opens();
    std::cout << "Legacy Star Vessel payload-cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
