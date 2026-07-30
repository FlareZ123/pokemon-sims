#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static std::optional<Card> choose_treasure_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true, Card::MysteriousTreasure);
  }
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

sim::State exact_t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::RoseannesBackup, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Grass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::BrilliantBlender};
  return state;
}

sim::Scenario exact_scenario(const sim::LockMode locks = sim::LockMode::None,
                             const int max_turn = 5) {
  return sim::Scenario{"issue-1765", sim::DciProfile::StrictJit, locks,
                       false, max_turn};
}

sim::Engine make_engine(sim::Scenario& scenario, sim::DeckRecipe& recipe,
                        std::mt19937_64& rng) {
  return sim::Engine(scenario, recipe, rng);
}

void test_exact_k0_state_spends_only_surplus_grass() {
  sim::Scenario scenario = exact_scenario();
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{176500};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_t2_state());

  // One held Grass is route-proven surplus after reserving one Grass and one Fire
  // for the two unused attachment windows. Treasure may spend that copy and search
  // the evolution card without using hidden Prize or draw-order knowledge:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Dynamic DCI and K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1765
  const auto cost = sim::EngineTestAccess::choose_treasure_cost(engine);
  expect(cost == sim::Card::Grass,
         "The exact K0 route did not admit only the surplus Grass Energy.");
}

void test_required_energy_and_attachment_windows_remain_protected() {
  sim::Scenario scenario = exact_scenario();
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{176501};
  sim::Engine engine = make_engine(scenario, recipe, rng);

  sim::State state = exact_t2_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::choose_treasure_cost(engine),
         "Treasure spent the final required Grass Energy.");

  state = exact_t2_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::choose_treasure_cost(engine),
         "Treasure ignored the missing Fire reserve.");

  state = exact_t2_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::choose_treasure_cost(engine),
         "Treasure counted a spent current attachment window.");

  sim::Scenario short_scenario = exact_scenario(sim::LockMode::None, 2);
  sim::Engine short_engine(short_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(short_engine, exact_t2_state());
  expect(!sim::EngineTestAccess::choose_treasure_cost(short_engine),
         "Treasure admitted surplus Energy without enough attachment windows.");
}

void test_route_identity_and_lock_gates() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{176502};

  sim::Scenario locked_scenario = exact_scenario(sim::LockMode::TurnTwoItem);
  sim::Engine locked_engine(locked_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(locked_engine, exact_t2_state());
  expect(!sim::EngineTestAccess::choose_treasure_cost(locked_engine),
         "Treasure bypassed scheduled Item lock.");

  sim::Scenario scenario = exact_scenario();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = exact_t2_state();
  state.active->entered_turn = state.turn;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::choose_treasure_cost(engine),
         "Treasure spent Energy for a Regidrago that cannot evolve this turn.");

  state = exact_t2_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::EarthenVessel));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::choose_treasure_cost(engine),
         "Treasure ignored the missing next-turn discard outlet.");

  state = exact_t2_state();
  state.discard = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                   sim::Card::RegidragoVstar};
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::RegidragoVstar),
                   state.deck.end());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::choose_treasure_cost(engine),
         "Treasure spent Energy when no VSTAR search target remained possible.");
}

void test_registered_seed_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1765 fixture is unavailable.");

  std::mt19937_64 rng{2026072801};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound seed must choose the observable surplus-Energy Treasure line,
  // evolve on T2, preserve GGF through the T2/T3 attachments, and use the existing
  // Legacy Star plus Brilliant Blender policy to establish a current-turn payload:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug and CI seed: https://github.com/FlareZ123/pokemon-sims/issues/1765 https://github.com/FlareZ123/pokemon-sims/pull/1768
  expect(outcome.first_ready_turn == 3,
         "Registered seed 2026072801 did not improve from T4 to T3.");
  expect(trace_contains(trace, "T2 | DISCARD") &&
             trace_contains(trace, "Grass Energy (Mysterious Treasure cost)") &&
             trace_contains(trace, "T2 | PLAY ITEM") &&
             trace_contains(trace, "Regidrago VSTAR") &&
             trace_contains(trace, "T2 | EVOLVE") &&
             !trace_contains(trace, "T2 | LEGACY STAR") &&
             trace_contains(trace, "T3 | Earthen Vessel") &&
             trace_contains(trace, "T3 | LEGACY STAR") &&
             trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "T3 | READY"),
         "The source-bound trace omitted a required issue-1765 route step.");
}
}  // namespace

int main() {
  test_exact_k0_state_spends_only_surplus_grass();
  test_required_energy_and_attachment_windows_remain_protected();
  test_route_identity_and_lock_gates();
  test_registered_seed_reaches_t3();
  return 0;
}
