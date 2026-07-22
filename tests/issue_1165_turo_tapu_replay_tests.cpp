#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_live(const Engine& engine) {
    return engine.turo_tapu_future_supporter_route_live();
  }
  static bool play_route(Engine& engine) {
    return engine.play_turo_tapu_future_supporter_route();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

struct Fixture {
  explicit Fixture(const sim::LockMode locks = sim::LockMode::None,
                   const int max_turn = 5)
      : scenario{"issue-1165", sim::DciProfile::StrictJit, locks, true, max_turn},
        recipe{sim::baseline_recipe()},
        rng{1165},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 2, 0, sim::Tool::None}};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::Fire,
                sim::Card::GoodraVstar, sim::Card::Lusamine};
  state.deck = {sim::Card::Gladion, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven};
  return state;
}

void expect_rejected(sim::State state, const std::string_view label,
                     const sim::LockMode locks = sim::LockMode::None,
                     const int max_turn = 5, const bool deck_seen = true,
                     const bool prizes_revealed = true) {
  Fixture fixture{locks, max_turn};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), deck_seen,
                                   prizes_revealed);
  if (sim::EngineTestAccess::route_live(fixture.engine) ||
      sim::EngineTestAccess::play_route(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": route must remain rejected");
  }
}

void test_returns_and_replays_tapu_for_future_supporter() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(), true, true);

  // Turo may return the Active Tapu, the powered prior-turn Regidrago V becomes
  // Active, and replaying Tapu from hand triggers Wonder Tag for the future Gladion:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1165
  if (!sim::EngineTestAccess::route_live(fixture.engine) ||
      !sim::EngineTestAccess::play_route(fixture.engine)) {
    throw std::runtime_error("the source-bounded Turo and Tapu replay route must resolve");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 2 || after.active->fire != 0 ||
      after.bench.size() != 1 || after.bench.front().card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::Gladion) ||
      !contains(after.discard, sim::Card::ProfessorTuro) || !after.supporter_used) {
    throw std::runtime_error("Turo must promote Regidrago V, replay Tapu, and bank Gladion");
  }
}

void test_requires_prior_turn_powered_regidrago() {
  sim::State state = route_state();
  state.bench.front().entered_turn = 3;
  expect_rejected(std::move(state), "same-turn Regidrago V");

  state = route_state();
  state.bench.front().grass = 1;
  expect_rejected(std::move(state), "one-Grass Regidrago V");
}

void test_requires_source_bounded_connectors() {
  sim::State state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Gladion));
  expect_rejected(std::move(state), "absent Gladion");

  state = route_state();
  state.prizes.clear();
  expect_rejected(std::move(state), "unprized Treasure");

  state = route_state();
  expect_rejected(std::move(state), "unknown deck", sim::LockMode::None, 5, false, true);
}

void test_requires_future_legal_window() {
  expect_rejected(route_state(), "Item lock", sim::LockMode::TurnTwoItem);
  expect_rejected(route_state(), "insufficient turns", sim::LockMode::None, 4);

  sim::State state = route_state();
  state.manual_energy_used = false;
  expect_rejected(std::move(state), "unused current attachment");
}

}  // namespace

int main() {
  test_returns_and_replays_tapu_for_future_supporter();
  test_requires_prior_turn_powered_regidrago();
  test_requires_source_bounded_connectors();
  test_requires_future_legal_window();
  return 0;
}
