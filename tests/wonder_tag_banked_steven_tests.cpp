#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static Card choose_supporter_after_search_started(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario strict_first_scenario(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-946-banked-steven", sim::DciProfile::StrictJit,
                       locks, true, 4};
}

sim::State turn_one_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::BrilliantBlender};
  state.deck = {
      sim::Card::StevensResolve,
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
      sim::Card::Arven,
  };
  return state;
}

sim::State turn_two_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire,
                sim::Card::BrilliantBlender};
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
      sim::Card::Arven,
  };
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, const std::uint64_t seed) {
  static sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng;
  rng.seed(seed);
  return sim::Engine(scenario, recipe, rng);
}

void wonder_tag_prefers_the_deterministic_t3_steven_route() {
  auto engine = make_engine(strict_first_scenario(), 94601);
  sim::EngineTestAccess::set_state(engine, turn_one_state());

  // Wonder Tag can bank a Supporter while the first player is prohibited from using
  // it on turn one. Steven then searches VSTAR, Crispin, and Latias for the T3 line:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/946
  expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not bank Steven's Resolve for the deterministic T3 route");
}

void turn_two_steven_reserves_every_t3_axis() {
  auto engine = make_engine(strict_first_scenario(), 94602);
  sim::EngineTestAccess::set_state(engine, turn_two_state());

  expect(sim::EngineTestAccess::should_play_steven(engine),
         "The banked Steven route was not recognized before the turn-two attachment");
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The turn-two Fire attachment was not made before Steven's Resolve");
  expect(sim::EngineTestAccess::should_play_steven(engine),
         "The banked Steven route disappeared after the turn-two attachment");
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven's Resolve did not execute the known T3 route");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  // Steven's unrestricted three-card search must reserve the Evolution, Energy, and
  // promotion axes while held Blender remains the strict-JIT payload outlet:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/946
  expect(contains(state.hand, sim::Card::RegidragoVstar),
         "Steven did not reserve Regidrago VSTAR");
  expect(contains(state.hand, sim::Card::Crispin),
         "Steven did not reserve Crispin for the final GGF component");
  expect(contains(state.hand, sim::Card::LatiasEx),
         "Steven did not reserve Latias ex for Active promotion");
  expect(contains(state.hand, sim::Card::BrilliantBlender),
         "The held Brilliant Blender payload outlet was lost");
  expect(state.turn_ended, "Steven's Resolve did not end the turn");
}

void future_selector_requires_every_proven_axis() {
  {
    auto engine = make_engine(strict_first_scenario(), 94603);
    sim::State state = turn_one_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                               sim::Card::BrilliantBlender));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) !=
               sim::Card::StevensResolve,
           "Wonder Tag selected Steven without the held Blender outlet");
  }
  {
    auto engine = make_engine(strict_first_scenario(sim::LockMode::TurnTwoItem),
                              94604);
    sim::EngineTestAccess::set_state(engine, turn_one_state());
    expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) !=
               sim::Card::StevensResolve,
           "Wonder Tag selected Steven through the scheduled Item lock");
  }
  {
    auto engine = make_engine(strict_first_scenario(), 94605);
    sim::State state = turn_one_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) !=
               sim::Card::StevensResolve,
           "Wonder Tag selected Steven without a Latias promotion target");
  }
  {
    auto engine = make_engine(strict_first_scenario(), 94606);
    sim::State state = turn_one_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                               sim::Card::Fire));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) !=
               sim::Card::StevensResolve,
           "Wonder Tag selected Steven without the two-turn manual Energy plan");
  }
}

void turn_two_route_rejects_missing_or_locked_continuations() {
  {
    auto engine = make_engine(strict_first_scenario(), 94607);
    sim::State state = turn_two_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                               sim::Card::BrilliantBlender));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::should_play_steven(engine),
           "Steven remained live without the held Blender continuation");
  }
  {
    auto engine = make_engine(strict_first_scenario(sim::LockMode::TurnTwoItem),
                              94608);
    sim::EngineTestAccess::set_state(engine, turn_two_state());
    expect(!sim::EngineTestAccess::should_play_steven(engine),
           "Steven remained live after scheduled Item lock removed Blender");
  }
  {
    auto engine = make_engine(strict_first_scenario(), 94609);
    sim::State state = turn_two_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                               sim::Card::LatiasEx));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::should_play_steven(engine),
           "Steven remained live without a Latias promotion target");
  }
}

}  // namespace

int main() {
  wonder_tag_prefers_the_deterministic_t3_steven_route();
  turn_two_steven_reserves_every_t3_axis();
  future_selector_requires_every_proven_axis();
  turn_two_route_rejects_missing_or_locked_continuations();
  return 0;
}
