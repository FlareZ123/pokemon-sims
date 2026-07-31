#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static std::optional<Card> payload_cost(const Engine& engine) {
    return engine.issue_1895_quick_ball_payload_cost();
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1895_held_crispin_quick_ball_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1895_held_crispin_quick_ball_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario strict(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1895", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::Crispin, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, std::move(state), deck_seen, prizes_revealed);
  return engine;
}

void test_exact_route_and_public_k1_boundaries() {
  std::mt19937_64 rng(1895);
  const sim::Scenario strict_scenario = strict();
  const sim::Scenario item_locked_scenario = strict(sim::LockMode::FullItem);
  sim::Engine engine = make_engine(strict_scenario, rng, route_state());

  // Quick Ball's Dragon cost is the current-turn Apex Dragon payload, while
  // held Crispin attaches the missing Grass from a two-type public-K1 deck.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1895
  expect(sim::EngineTestAccess::payload_cost(engine) == sim::Card::Dragapult,
         "Deck-search K1 did not select the held Dragon payload.");
  expect(sim::EngineTestAccess::route_available(engine),
         "The complete held-Crispin Quick Ball route was unavailable.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The complete held-Crispin Quick Ball route failed.");

  sim::Engine prize_k1 =
      make_engine(strict_scenario, rng, route_state(), false, true);
  expect(sim::EngineTestAccess::payload_cost(prize_k1) == sim::Card::Dragapult,
         "Prize-inspection K1 did not admit the Quick Ball payload cost.");

  sim::Engine k0 = make_engine(strict_scenario, rng, route_state(), false, false);
  expect(!sim::EngineTestAccess::payload_cost(k0).has_value(),
         "True K0 admitted the public-K1-only payload cost.");

  sim::Engine item_locked =
      make_engine(item_locked_scenario, rng, route_state());
  expect(!sim::EngineTestAccess::payload_cost(item_locked).has_value(),
         "Item lock admitted the Quick Ball route.");

  sim::State supporter_spent = route_state();
  supporter_spent.supporter_used = true;
  sim::Engine spent = make_engine(strict_scenario, rng, std::move(supporter_spent));
  expect(!sim::EngineTestAccess::payload_cost(spent).has_value(),
         "A spent Supporter action admitted the Crispin route.");

  sim::State one_type = route_state();
  one_type.deck.erase(std::find(one_type.deck.begin(), one_type.deck.end(),
                                sim::Card::Fire));
  sim::Engine missing_type = make_engine(strict_scenario, rng, std::move(one_type));
  expect(!sim::EngineTestAccess::payload_cost(missing_type).has_value(),
         "One searchable Energy type admitted the two-type Crispin finish.");

  sim::State no_target = route_state();
  no_target.deck = {sim::Card::Grass, sim::Card::Fire};
  sim::Engine targetless = make_engine(strict_scenario, rng, std::move(no_target));
  expect(!sim::EngineTestAccess::payload_cost(targetless).has_value(),
         "Quick Ball without a legal Basic target admitted the cost.");

  sim::State payload_done = route_state();
  payload_done.discard = {sim::Card::Dragapult};
  payload_done.discarded_this_turn = {sim::Card::Dragapult};
  sim::Engine done = make_engine(strict_scenario, rng, std::move(payload_done));
  expect(!sim::EngineTestAccess::payload_cost(done).has_value(),
         "An already-satisfied current-turn payload spent another Dragon.");
}

void test_registered_witnesses_reach_t3() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto strict_scenario = sim::scenario_by_label("strict-jit/go-first");
  const auto flex_scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  expect(deck != nullptr && strict_scenario.has_value() && flex_scenario.has_value(),
         "The registered issue-1895 fixtures are unavailable.");

  std::mt19937_64 strict_rng(951);
  sim::Engine strict_engine(*strict_scenario, deck->recipe, strict_rng);
  expect(strict_engine.run().first_ready_turn == 3,
         "Strict-JIT seed 951 did not reach readiness on T3.");

  std::mt19937_64 flex_rng(489);
  sim::Engine flex_engine(*flex_scenario, deck->recipe, flex_rng);
  expect(flex_engine.run().first_ready_turn == 3,
         "Matchup-flex seed 489 did not reach readiness on T3.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_public_k1_boundaries();
    test_registered_witnesses_reach_t3();
    std::cout << "Issue 1895 Quick Ball payload-cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
