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
    engine.prizes_revealed_ = true;
  }
  static bool late_steven_blender_route(const Engine& engine) {
    return engine.late_steven_vstar_with_held_blender_route_available();
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

sim::State issue_1109_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender,
                sim::Card::Gladion};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, sim::Card::Dragapult};
  state.prizes = {sim::Card::RegidragoV};
  return state;
}

sim::Scenario issue_1109_scenario(const sim::LockMode lock = sim::LockMode::None,
                                  const int max_turn = 5) {
  return sim::Scenario{"issue-1109-late-steven-blender",
                       sim::DciProfile::StrictJit, lock, false, max_turn};
}

void test_exact_k1_state_admits_route() {
  const sim::Scenario scenario = issue_1109_scenario();
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1109};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_k1_state(engine, issue_1109_state());

  // Steven reserves the known-searchable VSTAR. The prior-turn Active already has
  // GGF, and held Blender supplies the payload on the next strict-JIT ready turn:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1109
  expect(sim::EngineTestAccess::late_steven_blender_route(engine),
         "The exact K1 VSTAR-plus-held-Blender route must admit Steven.");
}

void test_required_route_components_gate_admission() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  {
    const sim::Scenario scenario = issue_1109_scenario();
    std::mt19937_64 rng{1110};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = issue_1109_state();
    std::erase(state.hand, sim::Card::BrilliantBlender);
    sim::EngineTestAccess::set_k1_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::late_steven_blender_route(engine),
           "Missing held Blender must reject the route.");
  }

  {
    const sim::Scenario scenario = issue_1109_scenario(sim::LockMode::FullItem);
    std::mt19937_64 rng{1111};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_k1_state(engine, issue_1109_state());
    // Blender is an Item, so full Item lock invalidates the next-turn outlet:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    expect(!sim::EngineTestAccess::late_steven_blender_route(engine),
           "Full Item lock must reject the held-Blender route.");
  }

  {
    const sim::Scenario scenario = issue_1109_scenario();
    std::mt19937_64 rng{1112};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = issue_1109_state();
    std::erase(state.deck, sim::Card::RegidragoVstar);
    std::erase(state.deck, sim::Card::RegidragoVstar);
    sim::EngineTestAccess::set_k1_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::late_steven_blender_route(engine),
           "A K1-proven missing VSTAR target must reject the route.");
  }
}

void test_seed_143_uses_steven_and_reaches_by_turn_four() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-second scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{143};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The complete Steven route must outrank spending Gladion on a redundant Basic.
  // A later policy may legally discover an earlier Steven plus Burnet continuation,
  // so this integration regression preserves the route and its original T4 deadline
  // while the exact-state test above continues to cover held Blender admission:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1109
  // https://github.com/FlareZ123/pokemon-sims/issues/1112
  const bool used_steven_by_t3 =
      trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-STEVEN-01") ||
      trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01");
  expect(used_steven_by_t3, "Seed 143 must play Steven by T3.");
  expect(!trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-GLADION-01") &&
             !trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-GLADION-01"),
         "Seed 143 must not spend T2 or T3 Gladion on a backup Basic.");
  expect(outcome.ready_by_4 && outcome.first_ready_turn <= 4,
         "Seed 143 must become ready no later than T4.");
}
}  // namespace

int main() {
  test_exact_k1_state_admits_route();
  test_required_route_components_gate_admission();
  test_seed_143_uses_steven_and_reaches_by_turn_four();
  return 0;
}
