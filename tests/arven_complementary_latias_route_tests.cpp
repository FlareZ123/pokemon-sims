#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State route_state(const int payload_count, const bool latias_in_deck) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0},
                 sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Arven, sim::Card::MysteriousTreasure,
                sim::Card::ProfessorBurnet, sim::Card::DialgaGX};
  if (payload_count > 1) state.hand.push_back(sim::Card::Dragapult);
  state.deck = {sim::Card::RegidragoVstar, sim::Card::QuickBall,
                sim::Card::EvolutionIncense, sim::Card::Grass,
                sim::Card::Fire, sim::Card::ForestSealStone};
  if (latias_in_deck) state.deck.push_back(sim::Card::LatiasEx);
  return state;
}

void chooses_complementary_quick_ball() {
  const sim::Scenario scenario{"issue-781-positive", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{781};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(2, true));
  // Treasure covers VSTAR; Quick Ball remains payable for Latias ex promotion:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/781
  if (!sim::EngineTestAccess::play_arven(engine)) throw std::runtime_error("Arven did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::QuickBall) ||
      !contains(after.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Arven duplicated Treasure instead of preserving the Latias connector.");
  }
}

void unpayable_second_cost_does_not_force_quick_ball() {
  const sim::Scenario scenario{"issue-781-unpayable", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7811};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(1, true));
  sim::EngineTestAccess::play_arven(engine);
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall)) {
    throw std::runtime_error("Arven claimed an unpayable sequential Item route.");
  }
}

void unavailable_latias_does_not_force_quick_ball() {
  const sim::Scenario scenario{"issue-781-no-latias", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7812};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(2, false));
  sim::EngineTestAccess::play_arven(engine);
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall)) {
    throw std::runtime_error("Arven searched Quick Ball without a Latias ex target.");
  }
}

void item_lock_blocks_complementary_item() {
  const sim::Scenario scenario{"issue-781-lock", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7813};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(2, true));
  // Arven remains a legal Supporter, while its searched Item route cannot resolve:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv1-166
  sim::EngineTestAccess::play_arven(engine);
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall)) {
    throw std::runtime_error("Item lock failed to block Arven's Item target.");
  }
}
}  // namespace

int main() {
  chooses_complementary_quick_ball();
  unpayable_second_cost_does_not_force_quick_ball();
  unavailable_latias_does_not_force_quick_ball();
  item_lock_blocks_complementary_item();
  return 0;
}
