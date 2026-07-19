#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool late_steven_route(const Engine& engine) {
    return engine.late_steven_vstar_vessel_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State late_steven_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::Channeler};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

void test_exact_k1_state_admits_late_steven_route() {
  const sim::Scenario scenario{"issue-1017-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1017};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_k1_state(engine, late_steven_state());

  // Steven reserves the missing VSTAR and Vessel. The following-turn Vessel has a
  // real Basic Energy target and discards the held Dragon on the strict-JIT ready turn:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1017
  expect(sim::EngineTestAccess::late_steven_route(engine),
         "The exact K1 VSTAR-Vessel continuation must admit late Steven.");
}

void test_item_lock_rejects_late_vessel_route() {
  const sim::Scenario scenario{"issue-1017-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, true, 5};
  std::mt19937_64 rng{1018};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_k1_state(engine, late_steven_state());

  // The route depends on playing Earthen Vessel next turn, so modeled Item lock
  // invalidates the complete continuation: https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1017
  expect(!sim::EngineTestAccess::late_steven_route(engine),
         "Scheduled Item lock must reject the late Vessel route.");
}

void test_missing_energy_target_rejects_vessel_route() {
  const sim::Scenario scenario{"issue-1017-no-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1019};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = late_steven_state();
  std::erase(state.deck, sim::Card::Grass);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));

  // Earthen Vessel cannot be selected as the promised payload outlet when K1 proves
  // that no Basic Energy remains for its search effect:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/1017
  expect(!sim::EngineTestAccess::late_steven_route(engine),
         "A known targetless Vessel must not justify late Steven.");
}

void test_horizon_rejects_unresolvable_next_turn_route() {
  const sim::Scenario scenario{"issue-1017-horizon", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  std::mt19937_64 rng{1020};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_k1_state(engine, late_steven_state());

  // Steven ends the current turn, so no continuation exists inside a horizon that
  // ends on this turn: https://api.pokemontcg.io/v2/cards/sm7-145
  // https://github.com/FlareZ123/pokemon-sims/issues/1017
  expect(!sim::EngineTestAccess::late_steven_route(engine),
         "The route must remain inside the configured simulation horizon.");
}

void test_seed_14_wonder_tag_selects_steven_and_reaches_t4() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{14};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Wonder Tag must select the complete Steven route, then Vessel must discard the
  // held Dragon after the Active Regidrago V evolves on T4:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1017
  expect(trace_contains(trace, "T3 | WONDER TAG | rules: R-TAPU-01 | Searched and revealed Steven's Resolve"),
         "Seed 14 must choose Steven instead of Burnet.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 14 must resolve late Steven on T3.");
  expect(trace_contains(trace, "T4 | DISCARD | rules: R-EV-01 | Mega Dragonite ex"),
         "Seed 14 must use Vessel as the strict-JIT payload outlet.");
  expect(trace_contains(trace, "T4 | READY"),
         "Seed 14 must reach readiness on T4.");
  expect(outcome.ready_by_4 && outcome.first_ready_turn == 4,
         "Seed 14 must be ready by exactly T4.");
}

void test_seed_38_executes_held_steven_and_reaches_t5() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{38};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The refined report covers an already-held Steven. It must search VSTAR on T4,
  // then the held Vessel discards a Dragon and reaches diagnostic readiness on T5:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1017#issuecomment-5015578916
  expect(trace_contains(trace, "T4 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 38 must execute the held Steven on T4.");
  expect(trace_contains(trace, "T5 | DISCARD | rules: R-EV-01 | Mega Dragonite ex"),
         "Seed 38 must use its held Vessel on T5.");
  expect(trace_contains(trace, "T5 | READY"),
         "Seed 38 must reach the deterministic T5 diagnostic state.");
  expect(outcome.ready_by_5 && outcome.first_ready_turn == 5,
         "Seed 38 must first become ready on T5.");
}
}  // namespace

int main() {
  test_exact_k1_state_admits_late_steven_route();
  test_item_lock_rejects_late_vessel_route();
  test_missing_energy_target_rejects_vessel_route();
  test_horizon_rejects_unresolvable_next_turn_route();
  test_seed_14_wonder_tag_selects_steven_and_reaches_t4();
  test_seed_38_executes_held_steven_and_reaches_t5();
  return 0;
}
