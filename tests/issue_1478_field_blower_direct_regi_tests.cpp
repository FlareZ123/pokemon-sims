#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_1478_t1_field_blower_direct_regi_route_available();
  }

  static bool play_route(Engine& engine) {
    return engine.issue_1478_play_t1_field_blower_direct_regi_route();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }

  static bool deck_seen(const Engine& engine) {
    return engine.deck_seen_;
  }

  static void set_trace(Engine& engine, TraceLog* trace) {
    engine.trace_ = trace;
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

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  const auto it = std::find_if(trace.lines.begin(), trace.lines.end(),
                               [&text](const std::string& line) {
                                 return line.find(text) != std::string::npos;
                               });
  return static_cast<std::size_t>(std::distance(trace.lines.begin(), it));
}

sim::State public_t1_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.hand = {sim::Card::MysteriousTreasure,
                sim::Card::Arven,
                sim::Card::Crispin,
                sim::Card::FieldBlower,
                sim::Card::BrilliantBlender,
                sim::Card::ForestSealStone,
                sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::LockMode lock, const int max_turn,
                        std::mt19937_64& rng) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const sim::Scenario scenario{
      "issue-1478", sim::DciProfile::StrictJit, lock, true, max_turn};
  return sim::Engine(scenario, deck->recipe, rng);
}

bool route_available_from_state(sim::State state, const sim::LockMode lock,
                                const int max_turn,
                                const std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  sim::Engine engine = make_engine(lock, max_turn, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::route_available(engine);
}

void test_public_route_exposes_setup_dead_field_blower() {
  // Field Blower has no target in this no-lock, no-Tool, no-Stadium state.
  // Mysterious Treasure may therefore spend it before inspection and establish
  // the Regidrago V line that reaches the repository's earliest T3 ready state:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI and K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1478
  expect(route_available_from_state(
             public_t1_route_state(), sim::LockMode::None, 3, 147801),
         "The complete public T3 route did not expose Field Blower as Treasure fuel.");
}

void test_route_boundaries_preserve_field_blower() {
  sim::State missing_fss = public_t1_route_state();
  missing_fss.hand.erase(std::find(missing_fss.hand.begin(),
                                   missing_fss.hand.end(),
                                   sim::Card::ForestSealStone));
  expect(!route_available_from_state(
             std::move(missing_fss), sim::LockMode::None, 3, 147802),
         "Field Blower became discardable without Forest Seal Stone.");

  sim::State stadium_target = public_t1_route_state();
  stadium_target.stadium = sim::Stadium::ChaoticSwell;
  expect(!route_available_from_state(
             std::move(stadium_target), sim::LockMode::None, 3, 147803),
         "Field Blower became discardable while a Stadium target was in play.");

  sim::State tool_target = public_t1_route_state();
  tool_target.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                   sim::Tool::ForestSealStone});
  expect(!route_available_from_state(
             std::move(tool_target), sim::LockMode::None, 3, 147804),
         "Field Blower became discardable while a Tool target was in play.");

  sim::State full_bench = public_t1_route_state();
  for (int index = 0; index < 5; ++index) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 0});
  }
  expect(!route_available_from_state(
             std::move(full_bench), sim::LockMode::None, 3, 147805),
         "Field Blower became discardable without a Regidrago Bench slot.");

  expect(!route_available_from_state(
             public_t1_route_state(), sim::LockMode::FullRuleBoxAbility,
             3, 147806),
         "The no-lock route crossed a Rule Box Ability lock.");
  expect(!route_available_from_state(
             public_t1_route_state(), sim::LockMode::None, 2, 147807),
         "Field Blower became discardable without the T3 action window.");
}

void test_route_pays_cost_before_k1_and_promotes_regidrago() {
  std::mt19937_64 rng(147808);
  sim::Engine engine = make_engine(sim::LockMode::None, 3, rng);
  sim::State state = public_t1_route_state();
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::EarthenVessel, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::Dragapult};
  state.prizes = {sim::Card::RegidragoV, sim::Card::RegidragoV,
                  sim::Card::RegidragoV, sim::Card::Powerglass,
                  sim::Card::Gladion, sim::Card::MegaDragonite};
  sim::TraceLog trace{true, {}};
  sim::EngineTestAccess::set_trace(engine, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_route(engine),
         "The complete issue-1478 route did not resolve.");
  const sim::State& resolved = sim::EngineTestAccess::state(engine);
  expect(sim::EngineTestAccess::deck_seen(engine),
         "Mysterious Treasure did not establish K1.");
  expect(resolved.active &&
             resolved.active->card == sim::Card::RegidragoV,
         "The route did not promote Regidrago V.");
  expect(resolved.manual_energy_used && resolved.retreat_used,
         "The route did not use the legal T1 attachment and retreat.");
  expect(std::count(resolved.discard.begin(), resolved.discard.end(),
                    sim::Card::FieldBlower) == 1,
         "Field Blower did not pay Mysterious Treasure's cost.");
  expect(std::count(resolved.discard.begin(), resolved.discard.end(),
                    sim::Card::Fire) == 1,
         "Oricorio's one-Energy Retreat Cost was not paid.");
  expect(trace_index(trace, "Field Blower (Mysterious Treasure") <
             trace_index(trace, "Mysterious Treasure: deck inspected"),
         "The route used K1 before paying Mysterious Treasure's cost.");
}

void test_seed_290_reaches_strict_jit_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1478 seed fixture is unavailable.");

  std::mt19937_64 rng(290);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact current-main failure has a complete legal line: Treasure spends
  // setup-dead Field Blower, Regidrago V becomes Active on T1, Forest Seal Stone
  // and Crispin establish VSTAR plus G,F on T2, and Arven into Earthen Vessel
  // supplies the final Grass before Brilliant Blender establishes strict-JIT:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1478
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 290 did not complete the legal strict-JIT T3 route.");
  expect(trace_contains(
             trace,
             "Field Blower (Mysterious Treasure direct-Regidrago cost)"),
         "Seed 290 did not spend setup-dead Field Blower.");
  expect(trace_contains(trace, "T1 | RETREAT"),
         "Seed 290 did not promote Regidrago V on T1.");
  expect(trace_contains(trace, "T2 | STAR ALCHEMY"),
         "Seed 290 did not use Forest Seal Stone for the VSTAR axis.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "Arven"),
         "Seed 290 did not use Arven for the final Energy route.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 290 did not complete strict-JIT readiness on T3.");
}

}  // namespace

int main() {
  test_public_route_exposes_setup_dead_field_blower();
  test_route_boundaries_preserve_field_blower();
  test_route_pays_cost_before_k1_and_promotes_regidrago();
  test_seed_290_reaches_strict_jit_t3();
  return 0;
}
