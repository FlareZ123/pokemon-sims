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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_1697_held_next_turn_manual_energy_route();
  }

  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

bool has(const std::vector<sim::Card>& zone, const sim::Card card) {
  return std::find(zone.begin(), zone.end(), card) != zone.end();
}

sim::State exact_state(const bool held_fire = true,
                       const bool held_crispin = true) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::Grass, sim::Card::Dipplin,
                sim::Card::FieldBlower, sim::Card::Gladion};
  if (held_fire) state.hand.push_back(sim::Card::Fire);
  if (held_crispin) state.hand.push_back(sim::Card::Crispin);
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::ForestSealStone};
  state.discard = {sim::Card::Dragapult};
  state.manual_energy_used = true;
  return state;
}

void test_seed_83_evolves_on_turn_two_and_preserves_crispin() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat2-erika-channeler");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1697 registered setup is unavailable.");

  std::mt19937_64 rng{83};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();

  // Gladion can take the known prized Forest Seal Stone, Star Alchemy can search
  // Regidrago VSTAR, and the prior-turn Regidrago V can evolve on T2. The held
  // Fire Energy then completes Apex Dragon's GGF cost by T3 without Crispin:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, Tool, VSTAR Power, manual attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 and resource-preserving earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1697
  expect(outcome.first_ready_turn == 3,
         "Seed 83 must preserve the earliest T3 ready turn.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(trace, "Exchanged Gladion for Forest Seal Stone"),
         "Seed 83 must take the known prized Forest Seal Stone on T2.");
  expect(trace_contains(trace, "T2 | STAR ALCHEMY") &&
             trace_contains(trace, "T2 | EVOLVE"),
         "Seed 83 must search and evolve Regidrago VSTAR on T2.");
  expect(trace_contains(trace,
                        "T3 | ATTACH | rules: R-GAME-ENERGY | Fire Energy manually"),
         "Seed 83 must finish GGF with the held Fire Energy on T3.");
  expect(!trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-CRISPIN-01"),
         "Seed 83 must not spend Crispin on T2.");
  expect(has(state.hand, sim::Card::Crispin),
         "Crispin must remain in hand at the T3 ready state.");
}

void test_exact_state_admits_fss_route() {
  const sim::Scenario scenario{"issue-1697-exact",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{16970};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, exact_state());

  expect(sim::EngineTestAccess::route_available(engine),
         "The exact K1 held-Energy route must be admitted.");
  expect(sim::EngineTestAccess::play_gladion(engine),
         "The exact K1 state must take Forest Seal Stone through Gladion.");
  expect(has(engine.state().hand, sim::Card::ForestSealStone) &&
             has(engine.state().hand, sim::Card::Crispin) &&
             has(engine.state().hand, sim::Card::Fire),
         "The route must recover Forest Seal Stone while preserving Crispin and Fire.");
}

void test_missing_held_energy_keeps_crispin_route() {
  const sim::Scenario scenario{"issue-1697-missing-energy",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{16971};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, exact_state(false, true));

  expect(!sim::EngineTestAccess::route_available(engine),
         "The held-Energy route must reject a missing final Energy.");
}

void test_strict_jit_rejects_cross_turn_payload_projection() {
  const sim::Scenario scenario{"issue-1697-strict-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{16972};
  sim::State state = exact_state();
  state.discarded_this_turn = {sim::Card::Dragapult};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::route_available(engine),
         "Strict JIT must not project a current-turn payload into next turn.");
}

void test_no_next_turn_rejects_delayed_attachment() {
  const sim::Scenario scenario{"issue-1697-horizon-control",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 2};
  std::mt19937_64 rng{16973};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, exact_state());

  expect(!sim::EngineTestAccess::route_available(engine),
         "The route must require a legal next turn for the held attachment.");
}

void test_missing_live_crispin_does_not_expand_issue_scope() {
  const sim::Scenario scenario{"issue-1697-crispin-control",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{16974};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, exact_state(true, false));

  expect(!sim::EngineTestAccess::route_available(engine),
         "The issue-1697 override must stay limited to preserving live Crispin.");
}

}  // namespace

int main() {
  try {
    test_seed_83_evolves_on_turn_two_and_preserves_crispin();
    test_exact_state_admits_fss_route();
    test_missing_held_energy_keeps_crispin_route();
    test_strict_jit_rejects_cross_turn_payload_projection();
    test_no_next_turn_rejects_delayed_attachment();
    test_missing_live_crispin_does_not_expand_issue_scope();
    std::cout << "Issue 1697 prized FSS held-Energy tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
