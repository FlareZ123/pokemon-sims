#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool value) { engine.deck_seen_ = value; }
  static bool should_play_steven(const Engine& engine) { return engine.should_play_steven(); }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State exact_turn_two_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::RoseannesBackup,
                sim::Card::MysteriousTreasure, sim::Card::Serena,
                sim::Card::Fire, sim::Card::MawileGX,
                sim::Card::Grass, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::ProfessorBurnet,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, sim::State state,
                        std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_turn_two_going_first_uses_only_basic_connector() {
  const sim::Scenario scenario{"issue-779", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{779};
  sim::Engine engine = make_engine(scenario, exact_turn_two_state(), rng);

  // Steven's Resolve may search Regidrago V on turn two. Its turn-ending clause
  // still leaves that Basic available to Bench on the next turn, while the held
  // singleton Mysterious Treasure has no policy-legal discard in this public hand:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/779
  if (!sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Late Steven should be eligible as the only Basic connector.");
  }
  if (!sim::EngineTestAccess::play_steven(engine)) {
    throw std::runtime_error("Late Steven should resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.turn_ended || !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Steven should search Regidrago V and end the turn.");
  }
}

void test_payable_mysterious_treasure_stays_faster() {
  const sim::Scenario scenario{"issue-779-payable-item", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{780};
  sim::State state = exact_turn_two_state();
  state.hand.push_back(sim::Card::MysteriousTreasure);
  sim::Engine engine = make_engine(scenario, std::move(state), rng);
  if (sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("A payable same-turn Treasure route must stay ahead of Steven.");
  }
}

void test_communication_requires_a_safe_exchange() {
  const sim::Scenario scenario{"issue-779-communication", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  {
    std::mt19937_64 rng{7801};
    sim::State state = exact_turn_two_state();
    state.hand.push_back(sim::Card::PokemonCommunication);
    state.hand.push_back(sim::Card::Dipplin);
    sim::Engine engine = make_engine(scenario, std::move(state), rng);
    // Pokémon Communication can return Dipplin and search Regidrago V immediately:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_pokemon_communication.inc#L42-L61
    if (sim::EngineTestAccess::should_play_steven(engine)) {
      throw std::runtime_error("A live Communication exchange must stay ahead of Steven.");
    }
  }
  {
    std::mt19937_64 rng{7802};
    sim::State state = exact_turn_two_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MawileGX),
                     state.hand.end());
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite),
                     state.hand.end());
    state.hand.push_back(sim::Card::PokemonCommunication);
    sim::Engine engine = make_engine(scenario, std::move(state), rng);
    if (!sim::EngineTestAccess::should_play_steven(engine)) {
      throw std::runtime_error("A dead Communication with no hand Pokémon must not suppress Steven.");
    }
  }
}

void test_item_lock_allows_only_supporter_route() {
  const sim::Scenario scenario{"issue-779-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, true, 4};
  std::mt19937_64 rng{781};
  sim::Engine engine = make_engine(scenario, exact_turn_two_state(), rng);
  if (!sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Item lock should preserve the legal Steven Basic route.");
  }
}

void test_existing_basic_and_no_bench_space_suppress_steven() {
  const sim::Scenario scenario{"issue-779-controls", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  {
    std::mt19937_64 rng{782};
    sim::State state = exact_turn_two_state();
    state.hand.push_back(sim::Card::RegidragoV);
    sim::Engine engine = make_engine(scenario, std::move(state), rng);
    if (sim::EngineTestAccess::should_play_steven(engine)) {
      throw std::runtime_error("A held Regidrago V must suppress the late Basic route.");
    }
  }
  {
    std::mt19937_64 rng{783};
    sim::State state = exact_turn_two_state();
    for (int i = 0; i < 5; ++i) state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0});
    sim::Engine engine = make_engine(scenario, std::move(state), rng);
    if (sim::EngineTestAccess::should_play_steven(engine)) {
      throw std::runtime_error("Steven should not fetch a Basic when no Bench space exists.");
    }
  }
}

void test_known_prized_basic_suppresses_late_route() {
  const sim::Scenario scenario{"issue-779-prized-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{785};
  sim::State state = exact_turn_two_state();
  state.deck.erase(state.deck.begin());
  state.prizes.push_back(sim::Card::RegidragoV);
  sim::Engine engine = make_engine(scenario, std::move(state), rng);
  sim::EngineTestAccess::set_deck_seen(engine, true);
  if (sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("A known-prized Regidrago V is not a Steven deck target.");
  }
}

void test_first_turn_going_first_remains_illegal() {
  const sim::Scenario scenario{"issue-779-first-turn", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{786};
  sim::State state = exact_turn_two_state();
  state.turn = 1;
  sim::Engine engine = make_engine(scenario, std::move(state), rng);
  if (sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("The first player cannot use Steven on the opening turn.");
  }
}

}  // namespace

int main() {
  test_turn_two_going_first_uses_only_basic_connector();
  test_payable_mysterious_treasure_stays_faster();
  test_communication_requires_a_safe_exchange();
  test_item_lock_allows_only_supporter_route();
  test_existing_basic_and_no_bench_space_suppress_steven();
  test_known_prized_basic_suppresses_late_route();
  test_first_turn_going_first_remains_illegal();
  return 0;
}
