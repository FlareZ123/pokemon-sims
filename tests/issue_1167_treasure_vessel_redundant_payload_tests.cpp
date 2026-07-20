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
    return engine.strict_dci_treasure_vessel_chain_live();
  }
  static bool play_route(Engine& engine) {
    return engine.play_strict_dci_treasure_vessel_chain();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

struct Fixture {
  explicit Fixture(const sim::LockMode locks = sim::LockMode::None,
                   const bool going_first = true, const int max_turn = 5)
      : scenario{"issue-1167", sim::DciProfile::StrictJit, locks,
                 going_first, max_turn},
        recipe{sim::baseline_recipe()},
        rng{1167},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::MegaDragonite,
                sim::Card::GoodraVstar, sim::Card::Lusamine,
                sim::Card::Fire, sim::Card::ProfessorTuro};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::Gladion, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Gladion, sim::Card::MysteriousTreasure,
                  sim::Card::QuickBall, sim::Card::QuickBall,
                  sim::Card::Arven, sim::Card::Oricorio};
  return state;
}

void expect_rejected(sim::State state, const std::string_view label,
                     const sim::LockMode locks = sim::LockMode::None,
                     const bool going_first = true, const int max_turn = 5,
                     const bool deck_seen = true,
                     const bool prizes_revealed = true) {
  Fixture fixture{locks, going_first, max_turn};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), deck_seen,
                                   prizes_revealed);
  if (sim::EngineTestAccess::route_live(fixture.engine) ||
      sim::EngineTestAccess::play_route(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": route must remain rejected");
  }
}

void test_pays_two_redundant_payloads_and_establishes_regi_gg() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(), true, true);

  // The two repeated Mega Dragonite ex copies pay Treasure and Vessel while the
  // distinct Goodra payload remains protected. The searches establish Regidrago V
  // and two Grass Energy without using any future draw identity:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1167
  if (!sim::EngineTestAccess::route_live(fixture.engine) ||
      !sim::EngineTestAccess::play_route(fixture.engine)) {
    throw std::runtime_error("the paired strict-DCI paid-search route must resolve");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.bench.size() != 1 || after.bench.front().card != sim::Card::RegidragoV ||
      count(after.hand, sim::Card::Grass) != 2 ||
      count(after.hand, sim::Card::GoodraVstar) != 1 ||
      count(after.hand, sim::Card::MegaDragonite) != 0 ||
      count(after.discard, sim::Card::MegaDragonite) != 2 ||
      count(after.discard, sim::Card::MysteriousTreasure) != 1 ||
      count(after.discard, sim::Card::EarthenVessel) != 1) {
    throw std::runtime_error("paired searches must preserve Goodra and establish Regidrago V plus two Grass");
  }
}

void test_requires_two_redundant_copies_and_distinct_payload() {
  sim::State state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite));
  expect_rejected(std::move(state), "one repeated payload");

  state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::GoodraVstar));
  expect_rejected(std::move(state), "no distinct protected payload");
}

void test_requires_both_search_targets_and_open_bench() {
  sim::State state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::RegidragoV));
  expect_rejected(std::move(state), "Regidrago V absent");

  state = route_state();
  auto grass = std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass);
  state.deck.erase(grass);
  grass = std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass);
  state.deck.erase(grass);
  expect_rejected(std::move(state), "fewer than two Grass");

  state = route_state();
  state.bench.assign(5, sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None});
  expect_rejected(std::move(state), "full Bench");
}

void test_requires_source_bounded_future_chain() {
  sim::State state = route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Gladion));
  expect_rejected(std::move(state), "Gladion absent");

  state = route_state();
  state.prizes.erase(std::find(state.prizes.begin(), state.prizes.end(), sim::Card::MysteriousTreasure));
  expect_rejected(std::move(state), "Treasure unprized");

  expect_rejected(route_state(), "unknown locations", sim::LockMode::None, true, 5, false, false);
}

void test_rejects_locks_wrong_seat_and_lower_dci_cost() {
  expect_rejected(route_state(), "Item lock", sim::LockMode::TurnTwoItem);
  expect_rejected(route_state(), "Rule Box lock", sim::LockMode::FullRuleBoxAbility);
  expect_rejected(route_state(), "going second", sim::LockMode::None, false);

  sim::State state = route_state();
  state.hand.push_back(sim::Card::Dipplin);
  expect_rejected(std::move(state), "ordinary lower-DCI fodder");
}

}  // namespace

int main() {
  test_pays_two_redundant_payloads_and_establishes_regi_gg();
  test_requires_two_redundant_copies_and_distinct_payload();
  test_requires_both_search_targets_and_open_bench();
  test_requires_source_bounded_future_chain();
  test_rejects_locks_wrong_seat_and_lower_dci_cost();
  return 0;
}
