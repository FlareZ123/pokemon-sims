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
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool available(const Engine& engine) {
    return engine.issue_1715_quick_ball_tapu_burnet_next_window_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

const sim::DeckRecipe& registered_shell_recipe() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) {
    throw std::runtime_error("The registered regidrago-shell recipe is unavailable.");
  }
  return deck->recipe;
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State exact_t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::Gladion, sim::Card::RoseannesBackup};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::ProfessorBurnet,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  state.manual_energy_used = true;
  state.supporter_used = true;
  return state;
}

sim::Scenario exact_scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1715", sim::DciProfile::StrictJit,
                       lock, false, 2};
}

void test_registered_seed_holds_and_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1715 registered fixture is unavailable.");
  std::mt19937_64 rng{777};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // K1 proves Quick Ball can spend one duplicate VSTAR for Tapu Lele-GX, whose
  // Wonder Tag finds Professor Burnet after the held Fire completes GGF on T2.
  // Celestial Roar cannot improve the legal evolution window and may discard a
  // required connector, so the deterministic route must be preserved:
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1715
  expect(outcome.first_ready_turn == 2,
         "Shell seed 777 did not retain its deterministic T2 route.");
  expect(trace_contains(trace, "T1 | HOLD ATTACK") &&
             !trace_contains(trace, "T1 | ATTACK |") &&
             trace_contains(trace, "Regidrago VSTAR (Quick Ball cost)") &&
             trace_contains(trace, "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(trace, "Searched and revealed Professor Burnet") &&
             trace_contains(trace, "T2 | READY"),
         "The source-bound seed 777 trace omitted a required issue-1715 step.");
}

void test_exact_k1_route_is_available() {
  std::mt19937_64 rng{17150};
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_t1_state());
  expect(sim::EngineTestAccess::available(engine),
         "The exact K1 Quick Ball-Tapu-Burnet continuation was rejected.");
}

void test_knowledge_cost_and_energy_gates() {
  std::mt19937_64 rng{17151};
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::EngineTestAccess::set_state(engine, exact_t1_state(), false);
  expect(!sim::EngineTestAccess::available(engine),
         "The route used deck identities before K1.");

  sim::State state = exact_t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::RegidragoVstar));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "A singleton VSTAR was exposed as Quick Ball's route cost.");

  state = exact_t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::QuickBall));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored a missing Quick Ball.");

  state = exact_t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Fire));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored the missing final Energy.");
}

void test_deck_connector_gates() {
  std::mt19937_64 rng{17152};
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine engine(scenario, recipe, rng);

  for (const sim::Card missing : {sim::Card::TapuLeleGX,
                                  sim::Card::ProfessorBurnet,
                                  sim::Card::MegaDragonite,
                                  sim::Card::Dragapult}) {
    sim::State state = exact_t1_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), missing));
    if (missing == sim::Card::MegaDragonite) {
      state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                                 sim::Card::Dragapult));
    }
    sim::EngineTestAccess::set_state(engine, state);
    const bool should_be_available = missing == sim::Card::Dragapult;
    expect(sim::EngineTestAccess::available(engine) == should_be_available,
           "A required K1 deck connector gate produced the wrong result.");
  }
}

void test_bench_and_lock_gates() {
  std::mt19937_64 bench_rng{17153};
  const sim::Scenario bench_scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine occupied(bench_scenario, recipe, bench_rng);
  sim::State state = exact_t1_state();
  for (int index = 0; index < 5; ++index) {
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  }
  sim::EngineTestAccess::set_state(occupied, state);
  expect(!sim::EngineTestAccess::available(occupied),
         "The route ignored a full Bench.");

  for (const sim::LockMode lock : {sim::LockMode::FullRuleBoxAbility,
                                   sim::LockMode::FullSupporter,
                                   sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullItem}) {
    std::mt19937_64 lock_rng{17154 + static_cast<unsigned>(lock)};
    const sim::Scenario locked_scenario = exact_scenario(lock);
    sim::Engine locked(locked_scenario, recipe, lock_rng);
    sim::EngineTestAccess::set_state(locked, exact_t1_state());
    expect(!sim::EngineTestAccess::available(locked),
           "The route ignored an applicable Item, Ability, or Supporter lock.");
  }
}
}  // namespace

int main() {
  try {
    test_registered_seed_holds_and_reaches_t2();
    test_exact_k1_route_is_available();
    test_knowledge_cost_and_energy_gates();
    test_deck_connector_gates();
    test_bench_and_lock_gates();
    std::cout << "Issue 1715 Celestial Roar Quick Ball-Tapu-Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
