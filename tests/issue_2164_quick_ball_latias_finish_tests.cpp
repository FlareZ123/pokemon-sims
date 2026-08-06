#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <functional>
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
  static void set_knowledge(Engine& engine, const bool known) {
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_2164_quick_ball_latias_finish_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_2164_quick_ball_latias_finish();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State exact_public_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 3, 2, 0, sim::Tool::None}};
  state.hand = {sim::Card::Grass, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar, sim::Card::Serena,
                sim::Card::BrilliantBlender, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::GoodraVstar};
  state.deck = {sim::Card::LatiasEx, sim::Card::TapuLeleGX,
                sim::Card::Crispin};
  state.prizes = {sim::Card::ProfessorTuro, sim::Card::MysteriousTreasure,
                  sim::Card::EarthenVessel, sim::Card::PathToPeak,
                  sim::Card::Fire, sim::Card::Lusamine};
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::DciProfile profile,
                   const sim::LockMode lock = sim::LockMode::None)
      : scenario{"issue-2164-focused", profile, lock, true, 5},
        recipe(sim::baseline_recipe()),
        rng(2164),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {
    sim::EngineTestAccess::set_state(engine, exact_public_state());
    sim::EngineTestAccess::set_knowledge(engine, true);
  }
};

void verify_complete_route(const sim::DciProfile profile) {
  Fixture fixture(profile);
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "the exact public K1 route was not recognized");
  expect(sim::EngineTestAccess::complete_route(fixture.engine),
         "the exact Quick Ball-Latias route did not complete");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(state.active && state.active->card == sim::Card::RegidragoVstar &&
             state.active->grass == 2 && state.active->fire == 1,
         "the GGF Regidrago VSTAR was not promoted Active");
  expect(contains(state.discard, sim::Card::QuickBall) &&
             contains(state.discard, sim::Card::MegaDragonite) &&
             contains(state.discarded_this_turn, sim::Card::MegaDragonite),
         "Quick Ball did not establish the current-turn Dragon payload");
  expect(contains(state.hand, sim::Card::Serena) &&
             contains(state.hand, sim::Card::BrilliantBlender),
         "the direct route consumed an unnecessary Supporter or ACE SPEC");
  expect(!state.supporter_used && state.manual_energy_used && state.retreat_used,
         "the direct route used an incorrect action channel");
  expect(trace_contains(fixture.trace,
                        "Quick Ball issue-2164 current-turn payload cost") &&
             trace_contains(fixture.trace, "Latias ex for the issue-2164") &&
             trace_contains(fixture.trace, "Latias ex gives the Basic Active"),
         "the route trace omitted its cost, search, or retreat provenance");

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dialga-GX, Basic Pokémon: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, earliest-route, strict-JIT, and DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2164
}

void verify_gate(const char* label,
                 const std::function<void(Fixture&)>& mutate) {
  Fixture fixture(sim::DciProfile::StrictJit);
  mutate(fixture);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error(std::string("issue-2164 gate failed: ") + label);
  }
}

void test_route_controls() {
  verify_gate("true K0", [](Fixture& fixture) {
    sim::EngineTestAccess::set_knowledge(fixture.engine, false);
  });
  verify_gate("Latias unavailable", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::LatiasEx),
                     state.deck.end());
    state.prizes.push_back(sim::Card::LatiasEx);
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("Bench full", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    while (state.bench.size() < 5U) {
      state.bench.push_back(sim::Pokemon{sim::Card::MawileGX, 1});
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("Regidrago entered this turn", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.bench.front().entered_turn = state.turn;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("GG incomplete", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.bench.front().grass = 1;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("Fire missing", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::Fire),
                     state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("VSTAR missing", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::RegidragoVstar),
                     state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("payload missing", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.hand.erase(std::remove_if(state.hand.begin(), state.hand.end(),
                                    sim::is_payload),
                     state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("manual attachment spent", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("retreat spent", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.retreat_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("Active not Dialga-GX", [](Fixture& fixture) {
    auto state = sim::EngineTestAccess::state(fixture.engine);
    state.active->card = sim::Card::TapuLeleGX;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  });
  verify_gate("Rule Box Ability lock", [](Fixture& fixture) {
    fixture.scenario.locks = sim::LockMode::FullRuleBoxAbility;
  });
  verify_gate("Item lock", [](Fixture& fixture) {
    fixture.scenario.locks = sim::LockMode::FullItem;
  });
  verify_gate("no-discard-control timing", [](Fixture& fixture) {
    fixture.scenario.dci = sim::DciProfile::NoDiscardControl;
  });
}

void exact_seed(const char* scenario_label) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2164 exact-seed fixture unavailable");

  std::mt19937_64 rng{69};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "seed 69 did not reach readiness on T4");
  expect(trace_contains(trace,
                        "Quick Ball issue-2164 current-turn payload cost") &&
             trace_contains(trace, "Latias ex for the issue-2164") &&
             trace_contains(trace, "T4 | READY"),
         "seed 69 omitted the deterministic Quick Ball-Latias finish");
  expect(!trace_contains(trace, "T4 | WONDER TAG") &&
             !trace_contains(trace, "T4 | PLAY SUPPORTER | Serena"),
         "seed 69 still consumed the weaker Tapu-Supporter route");
}

}  // namespace

int main() {
  verify_complete_route(sim::DciProfile::StrictJit);
  verify_complete_route(sim::DciProfile::MatchupFlexJit);
  test_route_controls();
  exact_seed("strict-jit/go-first");
  exact_seed("matchup-flex-jit/go-first");
  return 0;
}
