#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect_oricorio_active(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::Oricorio ||
      !contains(state.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error(message);
  }
}

void expect_dialga_active(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::DialgaGX ||
      !contains(state.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(message);
  }
}

sim::Engine make_engine(sim::State state, std::mt19937_64& rng) {
  const sim::Scenario scenario{"issue-792", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

sim::State unpayable_quick_ball_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

void test_unpayable_quick_ball_keeps_oricorio_active() {
  std::mt19937_64 rng{792};
  sim::Engine engine = make_engine(unpayable_quick_ball_hand(), rng);

  // Quick Ball must discard another card before searching a Basic Pokémon. At setup,
  // strict JIT protects both Dragon payloads and the singleton route resources, so the
  // held Quick Ball is not a live Regidrago V connector:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/792
  sim::EngineTestAccess::choose_opening_active(engine);
  expect_oricorio_active(sim::EngineTestAccess::state(engine),
                         "An unpayable Quick Ball must not preserve Oricorio in hand.");
}

void test_unpayable_mysterious_treasure_keeps_oricorio_active() {
  std::mt19937_64 rng{793};
  sim::State state = unpayable_quick_ball_hand();
  std::replace(state.hand.begin(), state.hand.end(), sim::Card::QuickBall,
               sim::Card::MysteriousTreasure);
  sim::Engine engine = make_engine(std::move(state), rng);

  // Mysterious Treasure also requires a real discard cost before its Dragon search:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/792
  sim::EngineTestAccess::choose_opening_active(engine);
  expect_oricorio_active(sim::EngineTestAccess::state(engine),
                         "An unpayable Mysterious Treasure must not count as a live route.");
}

void test_duplicate_quick_ball_preserves_positive_route() {
  std::mt19937_64 rng{794};
  sim::State state = unpayable_quick_ball_hand();
  std::replace(state.hand.begin(), state.hand.end(), sim::Card::TeamYellsCheer,
               sim::Card::QuickBall);
  sim::Engine engine = make_engine(std::move(state), rng);

  // A second Quick Ball is another card and may pay the first copy's printed cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/792
  sim::EngineTestAccess::choose_opening_active(engine);
  expect_dialga_active(sim::EngineTestAccess::state(engine),
                       "A payable duplicate Quick Ball route should preserve Vital Dance.");
}

void test_duplicate_mysterious_treasure_preserves_positive_route() {
  std::mt19937_64 rng{795};
  sim::State state = unpayable_quick_ball_hand();
  std::replace(state.hand.begin(), state.hand.end(), sim::Card::QuickBall,
               sim::Card::MysteriousTreasure);
  std::replace(state.hand.begin(), state.hand.end(), sim::Card::TeamYellsCheer,
               sim::Card::MysteriousTreasure);
  sim::Engine engine = make_engine(std::move(state), rng);

  // One Mysterious Treasure may discard the distinct second copy:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/792
  sim::EngineTestAccess::choose_opening_active(engine);
  expect_dialga_active(sim::EngineTestAccess::state(engine),
                       "A payable duplicate Treasure route should preserve Vital Dance.");
}
}  // namespace

int main() {
  test_unpayable_quick_ball_keeps_oricorio_active();
  test_unpayable_mysterious_treasure_keeps_oricorio_active();
  test_duplicate_quick_ball_preserves_positive_route();
  test_duplicate_mysterious_treasure_preserves_positive_route();
  return 0;
}
