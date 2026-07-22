#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_live(const Engine& engine) {
    return engine.strict_dci_treasure_vessel_chain_live();
  }
  static bool play_route(Engine& engine) {
    return engine.play_strict_dci_treasure_vessel_chain();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State appletun_route_state() {
  sim::State state;
  state.turn = 2;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::Appletun, sim::Card::Appletun,
                sim::Card::GoodraVstar, sim::Card::Lusamine,
                sim::Card::Fire, sim::Card::ProfessorTuro};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::Gladion, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Appletun};
  state.prizes = {sim::Card::Gladion, sim::Card::MysteriousTreasure,
                  sim::Card::QuickBall, sim::Card::QuickBall,
                  sim::Card::Arven, sim::Card::Oricorio};
  return state;
}

void test_two_redundant_appletun_pay_the_paired_searches() {
  using namespace sim;
  const Scenario scenario{"issue-1314/appletun-paired-cost", DciProfile::StrictJit,
                          LockMode::None, true, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{1314};
  Engine engine{scenario, recipe, rng};
  EngineTestAccess::set_state(engine, appletun_route_state());

  // Mysterious Treasure and Earthen Vessel each discard one redundant Appletun.
  // A distinct Goodra payload stays protected while the searches establish
  // Regidrago V and two Basic Grass Energy:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/issues/1314
  if (!EngineTestAccess::route_live(engine) || !EngineTestAccess::play_route(engine)) {
    throw std::runtime_error("The redundant-Appletun paid-search route was rejected.");
  }

  const State& after = EngineTestAccess::state(engine);
  if (after.bench.size() != 1 || after.bench.front().card != Card::RegidragoV ||
      count(after.hand, Card::Grass) != 2 || count(after.hand, Card::GoodraVstar) != 1 ||
      count(after.hand, Card::Appletun) != 0 || count(after.discard, Card::Appletun) != 2 ||
      count(after.discard, Card::MysteriousTreasure) != 1 ||
      count(after.discard, Card::EarthenVessel) != 1) {
    throw std::runtime_error("The paired searches did not preserve Goodra and spend both Appletun.");
  }
}

void test_redundant_appletun_still_requires_a_distinct_payload() {
  using namespace sim;
  const Scenario scenario{"issue-1314/no-distinct-payload", DciProfile::StrictJit,
                          LockMode::None, true, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{1315};
  Engine engine{scenario, recipe, rng};
  State state = appletun_route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), Card::GoodraVstar));
  EngineTestAccess::set_state(engine, std::move(state));

  // Dynamic DCI protects the last payload identity. Two repeated Appletun become
  // spendable only while a distinct modeled Apex Dragon payload remains:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1314
  if (EngineTestAccess::route_live(engine) || EngineTestAccess::play_route(engine)) {
    throw std::runtime_error("The route spent Appletun without a distinct protected payload.");
  }
}

}  // namespace

int main() {
  test_two_redundant_appletun_pay_the_paired_searches();
  test_redundant_appletun_still_requires_a_distinct_payload();
  std::cout << "Issue 1314 Appletun Treasure/Vessel tests passed.\n";
  return 0;
}
