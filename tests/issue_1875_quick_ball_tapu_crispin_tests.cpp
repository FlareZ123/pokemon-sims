#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1875_quick_ball_tapu_crispin_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1875_quick_ball_tapu_crispin_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::Scenario strict(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1875", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 0};
  state.hand = {sim::Card::Fire, sim::Card::QuickBall,
                sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Crispin,
                sim::Card::TapuLeleGX};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_held_payload_route_and_boundaries() {
  std::mt19937_64 rng(1875);
  const sim::Scenario scenario = strict();
  sim::Engine engine = make_engine(scenario, rng, base_state());

  // The held Dragon pays Quick Ball as the strict-JIT payload, Tapu Lele-GX
  // supplies Wonder Tag, and Crispin attaches the final Grass after the manual
  // Fire attachment. All decisions use the public K1 state.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1875
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact held-payload route was not available.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact held-payload route did not complete.");
  expect(engine.state().active->grass == 2 && engine.state().active->fire == 1,
         "The held-payload route did not complete GGF.");
  expect(std::find(engine.state().discarded_this_turn.begin(),
                   engine.state().discarded_this_turn.end(),
                   sim::Card::Dragapult) != engine.state().discarded_this_turn.end(),
         "Quick Ball did not create the current-turn payload.");

  sim::Engine k0 = make_engine(scenario, rng, base_state(), false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the K1-only route.");

  sim::State full_bench = base_state();
  full_bench.bench.assign(5, sim::Pokemon{sim::Card::RegidragoV, 1});
  sim::Engine benched = make_engine(scenario, rng, std::move(full_bench));
  expect(!sim::EngineTestAccess::route_available(benched),
         "A full Bench admitted the Tapu route.");

  sim::Engine item_locked = make_engine(strict(sim::LockMode::FullItem), rng,
                                        base_state());
  expect(!sim::EngineTestAccess::route_available(item_locked),
         "Item lock admitted the Quick Ball route.");

  sim::Engine ability_locked = make_engine(
      strict(sim::LockMode::FullRuleBoxAbility), rng, base_state());
  expect(!sim::EngineTestAccess::route_available(ability_locked),
         "Rule Box Ability lock admitted Wonder Tag.");

  sim::Engine supporter_locked = make_engine(
      strict(sim::LockMode::FullSupporter), rng, base_state());
  expect(!sim::EngineTestAccess::route_available(supporter_locked),
         "Supporter lock admitted Crispin.");

  sim::State spent_attachment = base_state();
  spent_attachment.manual_energy_used = true;
  sim::Engine spent = make_engine(scenario, rng, std::move(spent_attachment));
  expect(!sim::EngineTestAccess::route_available(spent),
         "A spent manual attachment admitted the route.");

  sim::State one_type = base_state();
  one_type.deck.erase(std::find(one_type.deck.begin(), one_type.deck.end(),
                                sim::Card::Fire));
  sim::Engine missing_type = make_engine(scenario, rng, std::move(one_type));
  expect(!sim::EngineTestAccess::route_available(missing_type),
         "One searchable Energy type admitted the two-type Crispin route.");
}

void test_blender_payload_route() {
  std::mt19937_64 rng(307);
  const sim::Scenario scenario = strict();
  sim::State state = base_state();
  state.hand = {sim::Card::Fire, sim::Card::QuickBall,
                sim::Card::BrilliantBlender, sim::Card::Klara};
  state.deck.push_back(sim::Card::Dragapult);
  sim::Engine engine = make_engine(scenario, rng, std::move(state));

  // Brilliant Blender supplies the current-turn payload, so route-replaced
  // Klara may pay Quick Ball while Crispin completes the Energy axis.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1875
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact Blender payload route was not available.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact Blender payload route did not complete.");
  expect(std::find(engine.state().discard.begin(), engine.state().discard.end(),
                   sim::Card::Klara) != engine.state().discard.end(),
         "The route-replaced future Supporter did not pay Quick Ball.");

  sim::State no_cost = base_state();
  no_cost.hand = {sim::Card::Fire, sim::Card::QuickBall,
                  sim::Card::BrilliantBlender};
  no_cost.deck.push_back(sim::Card::Dragapult);
  sim::Engine missing_cost = make_engine(scenario, rng, std::move(no_cost));
  expect(!sim::EngineTestAccess::route_available(missing_cost),
         "Blender admitted a route without a safe Quick Ball cost.");
}

void test_registered_witnesses_reach_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1875 fixtures are unavailable.");

  for (const std::uint64_t seed : {157ULL, 307ULL}) {
    std::mt19937_64 rng(seed);
    sim::TraceLog trace{true, {}};
    sim::Engine engine(*scenario, deck->recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();
    expect(outcome.first_ready_turn == 3,
           "A registered issue-1875 witness did not reach T3.");
    if (!(trace_contains(trace, "WONDER TAG") && trace_contains(trace, "Crispin"))) {
      std::cerr << "Seed " << seed << " trace:\n";
      for (const std::string& line : trace.lines) std::cerr << line << '\n';
      throw std::runtime_error("A registered issue-1875 witness did not use Tapu-Crispin.");
    }
  }
}
}  // namespace

int main() {
  try {
    test_held_payload_route_and_boundaries();
    test_blender_payload_route();
    test_registered_witnesses_reach_t3();
    std::cout << "Issue 1875 Quick Ball Tapu-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
