#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool compression_route(const Engine& engine) {
    return engine.late_steven_has_known_t3_compression_route();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void begin_turn(Engine& engine, const int turn) { engine.begin_turn(turn); }
  static bool ready(const Engine& engine) {
    const State& state = engine.state_;
    return state.turn >= 2 && engine.active_is_vstar() &&
        state.active->grass >= 2 && state.active->fire >= 1 && engine.payload_ready();
  }
  static std::string state_line(const Engine& engine) {
    return engine.state_line();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::Pokemon* find_benched(const sim::State& state, const sim::Card card) {
  const auto it = std::find_if(state.bench.begin(), state.bench.end(),
                               [card](const sim::Pokemon& pokemon) {
                                 return pokemon.card == card;
                               });
  return it == state.bench.end() ? nullptr : &*it;
}

sim::State exact_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Fire, sim::Card::TateLiza,
                sim::Card::Grass, sim::Card::QuickBall, sim::Card::StevensResolve};
  state.deck = {sim::Card::LatiasEx, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Dipplin,
                sim::Card::Dragapult, sim::Card::GoodraVstar,
                sim::Card::Grass, sim::Card::Fire, sim::Card::Channeler};
  state.prizes = {sim::Card::QuickBall, sim::Card::ForestSealStone,
                  sim::Card::PathToPeak, sim::Card::Crispin,
                  sim::Card::Grass, sim::Card::RegidragoV};
  return state;
}

struct EngineFixture {
  sim::Scenario scenario{"issue-869-late-steven-t3", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit EngineFixture(const std::uint64_t seed)
      : rng(seed), engine(scenario, recipe, rng) {}
};

// Engine stores references to Scenario, DeckRecipe, and RNG. Keep those owners alive
// for the entire fixture instead of returning an Engine bound to stack locals:
// https://github.com/FlareZ123/pokemon-sims/issues/869

void exact_route_reaches_turn_three() {
  EngineFixture fixture(869);
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::state(engine) = exact_route_state();
  sim::EngineTestAccess::set_deck_seen(engine);

  // Steven searches any three cards and ends the turn. The prior-turn Regidrago V
  // may evolve first; Fire is attached now, Grass next turn, Mysterious Treasure
  // discards Mega Dragonite ex, and Latias ex frees the Basic Active's Retreat Cost:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/issues/869
  if (!sim::EngineTestAccess::compression_route(engine) ||
      !sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("confirmed late-Steven route was not admitted");
  }

  sim::EngineTestAccess::run_turn(engine);
  sim::State& after_steven = sim::EngineTestAccess::state(engine);
  const sim::Pokemon* prepared = find_benched(after_steven, sim::Card::RegidragoVstar);
  if (!after_steven.turn_ended || !after_steven.supporter_used || prepared == nullptr ||
      prepared->grass != 1 || prepared->fire != 1 ||
      !contains(after_steven.hand, sim::Card::Grass) ||
      !contains(after_steven.hand, sim::Card::LatiasEx) ||
      !contains(after_steven.hand, sim::Card::MysteriousTreasure) ||
      !contains(after_steven.hand, sim::Card::MegaDragonite) ||
      !contains(after_steven.discard, sim::Card::StevensResolve)) {
    throw std::runtime_error(
        "Steven did not preserve the exact T3 route: " +
        sim::EngineTestAccess::state_line(engine));
  }

  sim::EngineTestAccess::begin_turn(engine, 3);
  if (!sim::EngineTestAccess::state(engine).turn_ended) {
    sim::EngineTestAccess::run_turn(engine);
  }
  const sim::State& ready = sim::EngineTestAccess::state(engine);
  if (!sim::EngineTestAccess::ready(engine) || !ready.active ||
      ready.active->card != sim::Card::RegidragoVstar ||
      ready.active->grass != 2 || ready.active->fire != 1 ||
      !contains(ready.discarded_this_turn, sim::Card::MegaDragonite) ||
      find_benched(ready, sim::Card::LatiasEx) == nullptr) {
    throw std::runtime_error(
        "confirmed Steven route did not reach readiness on T3: " +
        sim::EngineTestAccess::state_line(engine));
  }
}

void missing_target_blocks_route() {
  EngineFixture fixture(8691);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::LatiasEx),
                   state.deck.end());
  state.prizes.push_back(sim::Card::LatiasEx);
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine) ||
      sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven claimed a route with prized Latias ex");
  }
}

void full_bench_blocks_route() {
  EngineFixture fixture(8692);
  sim::Engine& engine = fixture.engine;
  sim::State state = exact_route_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::state(engine) = std::move(state);
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine) ||
      sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven claimed a Latias route without Bench space");
  }
}

void item_lock_blocks_next_turn_treasure() {
  const sim::Scenario scenario{"issue-869-item-lock-control", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{8693};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::state(engine) = exact_route_state();
  sim::EngineTestAccess::set_deck_seen(engine);
  if (sim::EngineTestAccess::compression_route(engine) ||
      sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven claimed an Item-dependent route under Item lock");
  }
}

}  // namespace

int main() {
  try {
    exact_route_reaches_turn_three();
    missing_target_blocks_route();
    full_bench_blocks_route();
    item_lock_blocks_next_turn_treasure();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
