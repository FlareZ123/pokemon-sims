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
    return engine.issue_1719_earthen_vessel_next_window_route();
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

sim::State exact_t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::Dragapult,
                sim::Card::QuickBall, sim::Card::Gladion,
                sim::Card::ChaoticSwell, sim::Card::ErikasInvitation,
                sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure, sim::Card::Grass,
                sim::Card::Fire};
  state.manual_energy_used = true;
  state.supporter_used = true;
  return state;
}

sim::Scenario exact_scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1719", sim::DciProfile::StrictJit,
                       lock, true, 3};
}

void test_registered_seed_holds_and_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1719 registered fixture is unavailable.");
  std::mt19937_64 rng{42};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The K1 T2 state holds a payable Earthen Vessel and a Dragon payload. Vessel
  // supplies the missing Grass and establishes strict-JIT on T3. Celestial Roar
  // cannot improve the marginal next-draw chance of a live VSTAR axis and may
  // discard the VSTAR or its search outs, so the random attack must be held:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Observable-state, K1, DCI/JIT, and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1719
  expect(outcome.first_ready_turn == 3,
         "Shell seed 42 did not preserve its earliest T3 continuation.");
  expect(trace_contains(trace, "T2 | HOLD ATTACK") &&
             !trace_contains(trace, "T2 | ATTACK |") &&
             (trace_contains(trace, "T3 | Earthen Vessel") ||
              trace_contains(trace, "T3 | EARTHEN VESSEL") ||
              trace_contains(trace, "T3 | PLAY ITEM | rules: R-EV-01")) &&
             trace_contains(trace, "T3 | READY"),
         "The source-bound seed 42 trace omitted a required issue-1719 step.");
}

void test_exact_k1_route_is_available() {
  std::mt19937_64 rng{17190};
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_t2_state());
  expect(sim::EngineTestAccess::available(engine),
         "The exact K1 Earthen Vessel continuation was rejected.");
}

void test_knowledge_item_cost_and_attachment_gates() {
  std::mt19937_64 rng{17191};
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::EngineTestAccess::set_state(engine, exact_t2_state(), false);
  expect(!sim::EngineTestAccess::available(engine),
         "The route used deck identities before K1.");

  sim::State state = exact_t2_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::EarthenVessel));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored a missing Earthen Vessel.");

  state = exact_t2_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Dragapult));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored the missing strict-JIT payload cost.");

  state = exact_t2_state();
  state.manual_energy_used = false;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored the unresolved current manual attachment window.");
}

void test_energy_and_vstar_axis_gates() {
  std::mt19937_64 rng{17192};
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state = exact_t2_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored the missing searched Grass Energy.");

  state = exact_t2_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::RegidragoVstar),
                   state.deck.end());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored a dead VSTAR axis.");

  state = exact_t2_state();
  state.active->grass = 0;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "The route ignored two missing Energy attachments.");
}

void test_item_lock_gates() {
  const sim::DeckRecipe& recipe = registered_shell_recipe();
  for (const sim::LockMode lock : {sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullItem,
                                   sim::LockMode::FullCombined}) {
    std::mt19937_64 rng{17193 + static_cast<unsigned>(lock)};
    const sim::Scenario scenario = exact_scenario(lock);
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, exact_t2_state());
    expect(!sim::EngineTestAccess::available(engine),
           "The route ignored an applicable next-turn Item lock.");
  }
}
}  // namespace

int main() {
  try {
    test_registered_seed_holds_and_reaches_t3();
    test_exact_k1_route_is_available();
    test_knowledge_item_cost_and_attachment_gates();
    test_energy_and_vstar_axis_gates();
    test_item_lock_gates();
    std::cout << "Issue 1719 Celestial Roar Vessel continuation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
