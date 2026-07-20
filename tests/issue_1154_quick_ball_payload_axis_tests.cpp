#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static std::optional<Card> final_surplus_cost(const Engine& engine) {
    return engine.quick_ball_final_surplus_energy_cost();
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng, sim::State state) {
  static const sim::Scenario scenario{"issue-1154", sim::DciProfile::MatchupFlexJit,
                                      sim::LockMode::None, false, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_payload_already_ready() {
  std::mt19937_64 rng{115400};
  sim::State state = route_state();
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::Engine engine = make_engine(rng, std::move(state));

  // A current-turn Dragon already satisfies matchup-flex JIT. Quick Ball may spend
  // the surplus Grass to search Latias ex and complete only the Active-position axis:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1154
  expect(sim::EngineTestAccess::final_surplus_cost(engine) == sim::Card::Grass,
         "A current-turn ready payload must admit the final surplus Energy cost.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must play when the payload axis is already ready.");
}

void test_held_burnet_route() {
  std::mt19937_64 rng{115401};
  sim::State state = route_state();
  state.hand.push_back(sim::Card::ProfessorBurnet);
  sim::Engine engine = make_engine(rng, std::move(state));

  // Professor Burnet can search and discard a Dragon after Quick Ball finds Latias ex,
  // preserving the one-Supporter limit and completing the same-turn payload axis:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1154
  expect(sim::EngineTestAccess::final_surplus_cost(engine) == sim::Card::Grass,
         "Held live Burnet must admit the final surplus Energy cost.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must play when held Burnet completes the payload axis.");
}

void test_negative_controls() {
  std::mt19937_64 rng1{115402};
  sim::State spent_supporter = route_state();
  spent_supporter.hand.push_back(sim::Card::ProfessorBurnet);
  spent_supporter.supporter_used = true;
  sim::Engine engine1 = make_engine(rng1, std::move(spent_supporter));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine1),
         "Burnet must not count after the Supporter play is spent.");

  std::mt19937_64 rng2{115403};
  sim::State old_payload = route_state();
  old_payload.discard = {sim::Card::MegaDragonite};
  old_payload.deck = {sim::Card::LatiasEx};
  sim::Engine engine2 = make_engine(rng2, std::move(old_payload));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine2),
         "A prior-turn payload must not satisfy matchup-flex JIT.");

  std::mt19937_64 rng3{115404};
  sim::State no_payload = route_state();
  no_payload.hand.push_back(sim::Card::ProfessorBurnet);
  no_payload.deck = {sim::Card::LatiasEx};
  sim::Engine engine3 = make_engine(rng3, std::move(no_payload));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine3),
         "Burnet must require a permitted Dragon in the inspected deck.");
}
}  // namespace

int main() {
  try {
    test_payload_already_ready();
    test_held_burnet_route();
    test_negative_controls();
    std::cout << "Issue 1154 Quick Ball payload-axis tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
