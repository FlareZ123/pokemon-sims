#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool direct_route(const Engine& engine) {
    return engine.issue_1412_direct_fss_fire_route_visible();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

struct Fixture {
  sim::Scenario scenario{"issue-1412/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 3};
  sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1412};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State complete_fss_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone,
                sim::Card::QuickBall,
                sim::Card::QuickBall};
  state.deck = {sim::Card::Fire, sim::Card::Pineco};
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  return state;
}

void test_complete_fss_route_suppresses_redundant_quick_ball() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine,
                                   complete_fss_route_state());

  // Forest Seal Stone plus the unused manual attachment and held VSTAR complete
  // every current T2 axis. Quick Ball must retain its discrete future value and
  // Pineco must not consume a Bench slot after Exploding Energy supplied GG:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1412
  expect(sim::EngineTestAccess::direct_route(fixture.engine),
         "The complete Forest Seal Stone route was not recognized.");
}

void test_missing_direct_axis_keeps_pineco_live() {
  const auto expect_incomplete = [](sim::State state, const char* message) {
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::direct_route(fixture.engine), message);
  };

  sim::State no_fss = complete_fss_route_state();
  no_fss.hand.erase(std::find(no_fss.hand.begin(), no_fss.hand.end(),
                              sim::Card::ForestSealStone));
  expect_incomplete(std::move(no_fss),
                    "Pineco was suppressed without Forest Seal Stone.");

  sim::State no_vstar = complete_fss_route_state();
  no_vstar.hand.erase(std::find(no_vstar.hand.begin(), no_vstar.hand.end(),
                                sim::Card::RegidragoVstar));
  expect_incomplete(std::move(no_vstar),
                    "Pineco was suppressed without the VSTAR evolution axis.");

  sim::State manual_used = complete_fss_route_state();
  manual_used.manual_energy_used = true;
  expect_incomplete(std::move(manual_used),
                    "Pineco was suppressed after the manual attachment was spent.");

  sim::State no_payload = complete_fss_route_state();
  no_payload.discard.clear();
  no_payload.discarded_this_turn.clear();
  expect_incomplete(std::move(no_payload),
                    "Pineco was suppressed without a current-turn strict-JIT payload.");

  sim::State new_regi = complete_fss_route_state();
  new_regi.active->entered_turn = 2;
  expect_incomplete(std::move(new_regi),
                    "Pineco was suppressed when the Active could not evolve this turn.");
}

void test_seed_73_preserves_both_quick_balls_and_bench_slot() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1412 seed fixture is unavailable.");

  std::mt19937_64 rng(73);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& after = sim::EngineTestAccess::state(engine);

  // Exact source-bound regression. Secret Box already discarded Dragapult ex,
  // Exploding Energy supplied GG, and Star Alchemy searched Fire. The corrected
  // route reaches the same T2 deadline while preserving both Quick Balls and the
  // second Pineco Bench slot:
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1412
  expect(outcome.first_ready_turn == 2,
         "Seed 73 lost its legal strict-JIT T2 ready state.");
  if (count(after.hand, sim::Card::QuickBall) != 2) {
    for (const std::string& line : trace.lines) std::cerr << line << '\n';
    std::cerr << "Final Quick Ball counts: hand="
              << count(after.hand, sim::Card::QuickBall)
              << " discard=" << count(after.discard, sim::Card::QuickBall)
              << '\n';
    throw std::runtime_error(
        "Seed 73 failed to preserve both Quick Ball copies.");
  }
  expect(std::none_of(after.bench.begin(), after.bench.end(),
                      [](const sim::Pokemon& pokemon) {
                        return pokemon.card == sim::Card::Pineco;
                      }),
         "Seed 73 still Benched the redundant second Pineco.");
  expect(count(after.hand, sim::Card::EarthenVessel) == 1,
         "Seed 73 spent Earthen Vessel after the direct FSS route was public.");
  expect(!trace_contains(trace, "Quick Ball (Quick Ball cost)"),
         "Seed 73 still paid the redundant Quick Ball cost.");
  expect(trace_contains(trace, "T2 | READY"),
         "Seed 73 trace no longer records T2 readiness.");
}

}  // namespace

int main() {
  try {
    test_complete_fss_route_suppresses_redundant_quick_ball();
    test_missing_direct_axis_keeps_pineco_live();
    test_seed_73_preserves_both_quick_balls_and_bench_slot();
    std::cout << "Issue 1412 Quick Ball preservation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
