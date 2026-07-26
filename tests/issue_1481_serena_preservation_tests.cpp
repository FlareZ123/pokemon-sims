#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }

  static bool hold_serena(const Engine& engine) {
    return engine.issue_1481_hold_serena_for_free_latias_completion();
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

sim::State complete_free_retreat_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1},
      sim::Pokemon{sim::Card::LatiasEx, 4}};
  state.hand = {sim::Card::Serena};
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  state.stadium = sim::Stadium::ChaoticSwell;
  state.path_lock_removed = true;
  return state;
}

bool hold_from_state(sim::State state, const sim::LockMode lock,
                     const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const sim::Scenario scenario{
      "issue-1481", sim::DciProfile::StrictJit, lock, false, 5};
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::hold_serena(engine);
}

void test_complete_free_retreat_holds_serena() {
  // Serena requires a real discard, while Skyliner already gives the Basic Active
  // a zero Retreat Cost and the Benched VSTAR has GGF plus a current-turn payload:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1481
  expect(hold_from_state(complete_free_retreat_state(),
                         sim::LockMode::FullRuleBoxAbility, 148101),
         "The complete Skyliner route did not preserve Serena.");
}

void test_active_regidrago_v_also_holds_serena() {
  sim::State state = complete_free_retreat_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};

  // Skyliner says every Basic Pokemon in play has no Retreat Cost. Active
  // Regidrago V therefore has the same free promotion route as Tapu Lele-GX:
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Core retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Underfix review: https://github.com/FlareZ123/pokemon-sims/pull/1484#issuecomment-5071540525
  expect(hold_from_state(std::move(state), sim::LockMode::FullRuleBoxAbility,
                         148107),
         "Active Regidrago V was incorrectly excluded from Skyliner.");
}

void test_boundaries_keep_live_serena_routes() {
  sim::State no_latias = complete_free_retreat_state();
  no_latias.bench.pop_back();
  expect(!hold_from_state(std::move(no_latias), sim::LockMode::None, 148102),
         "Serena was held without Latias ex in play.");

  sim::State locked_latias = complete_free_retreat_state();
  locked_latias.stadium = sim::Stadium::None;
  locked_latias.path_lock_removed = false;
  expect(!hold_from_state(std::move(locked_latias),
                          sim::LockMode::FullRuleBoxAbility, 148103),
         "Serena was held while Skyliner was locked.");

  sim::State missing_energy = complete_free_retreat_state();
  missing_energy.bench.front().grass = 1;
  expect(!hold_from_state(std::move(missing_energy), sim::LockMode::None, 148104),
         "Serena was held while the Benched VSTAR lacked GGF.");

  sim::State missing_payload = complete_free_retreat_state();
  missing_payload.discarded_this_turn.clear();
  missing_payload.hand.push_back(sim::Card::MegaDragonite);
  expect(!hold_from_state(std::move(missing_payload), sim::LockMode::None, 148105),
         "Serena's live strict-JIT payload route was suppressed.");

  sim::State retreat_spent = complete_free_retreat_state();
  retreat_spent.retreat_used = true;
  expect(!hold_from_state(std::move(retreat_spent), sim::LockMode::None, 148106),
         "Serena was held after the Retreat action was spent.");
}

void test_seed_8_preserves_serena_and_reaches_t4() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-rulebox-ability-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-8 fixture is unavailable.");

  std::mt19937_64 rng(8);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Brilliant Blender and Skyliner already complete the deterministic T4 route.
  // Serena remains unused because its mandatory discard advances no setup axis:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1481
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 8 did not retain its earliest T4 ready state.");
  expect(!trace_contains(trace, "Serena chosen discard") &&
             !trace_contains(trace, "Used Serena draw mode"),
         "Seed 8 still spent Serena after setup was deterministic.");
  expect(trace_contains(trace, "T4 | HOLD SUPPORTER") &&
             trace_contains(trace, "T4 | RETREAT") &&
             trace_contains(trace, "T4 | READY"),
         "Seed 8 did not follow the preserved-Serena Skyliner finish.");
}

}  // namespace

int main() {
  test_complete_free_retreat_holds_serena();
  test_active_regidrago_v_also_holds_serena();
  test_boundaries_keep_live_serena_routes();
  test_seed_8_preserves_serena_and_reaches_t4();
  return 0;
}
