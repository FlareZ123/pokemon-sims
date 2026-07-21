#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstddef>
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
  static bool preserve_before_evolution(const Engine& engine) {
    return engine.tate_switch_after_incomplete_held_vstar_evolution_route();
  }
  static bool switch_after_evolution(const Engine& engine) {
    return engine.tate_switch_after_incomplete_vstar_evolution_ready();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool evolve(Engine& engine) { return engine.evolve_best_regi(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Serena,
                sim::Card::TeamYellsCheer, sim::Card::TateLiza};
  state.deck = {sim::Card::Grass, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

std::size_t trace_index(const sim::TraceLog& trace, const int turn,
                        const std::string& action,
                        const std::string& detail) {
  const std::string prefix = "T" + std::to_string(turn) + " | ";
  for (std::size_t index = 0; index < trace.lines.size(); ++index) {
    const std::string& line = trace.lines[index];
    if (line.starts_with(prefix) && line.find(action) != std::string::npos &&
        line.find(detail) != std::string::npos) {
      return index;
    }
  }
  return trace.lines.size();
}

void test_exact_evolve_then_switch_order() {
  const sim::Scenario scenario{"issue-1197-exact", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{119700};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario, rng, route_state(), &trace);

  // A prior-turn Benched Regidrago V at GF may evolve with the held VSTAR, then
  // Tate & Liza may switch that evolved attacker Active. Those public, held axes
  // must resolve before Tate's random five-card redraw:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Evolution, switching, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest direct route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1197
  expect(sim::EngineTestAccess::preserve_before_evolution(engine),
         "The held GF evolve-switch route was not recognized.");
  sim::EngineTestAccess::choose_supporter(engine);
  expect(!engine.state().supporter_used,
         "Tate draw mode ran before the held VSTAR evolved.");
  expect(sim::EngineTestAccess::evolve(engine),
         "The prior-turn Benched Regidrago V did not evolve.");
  expect(sim::EngineTestAccess::switch_after_evolution(engine),
         "The evolved GF VSTAR was not recognized as the Tate switch target.");
  sim::EngineTestAccess::choose_supporter(engine);
  expect(engine.state().supporter_used,
         "Tate switch mode did not consume the Supporter play.");
  expect(engine.state().active &&
             engine.state().active->card == sim::Card::RegidragoVstar,
         "Tate did not promote the evolved Regidrago VSTAR.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::Serena) != engine.state().hand.end(),
         "The held Serena continuation was randomized.");
}

void test_route_boundaries() {
  const sim::Scenario strict{"issue-1197-controls", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  const auto rejected = [&](sim::State state, const sim::Scenario& scenario,
                            const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::preserve_before_evolution(engine), message);
  };

  sim::State same_turn = route_state();
  same_turn.bench.front().entered_turn = same_turn.turn;
  rejected(std::move(same_turn), strict, 119701,
           "A same-turn Benched Basic must not be treated as evolution-eligible.");

  sim::State no_vstar = route_state();
  no_vstar.hand.erase(std::remove(no_vstar.hand.begin(), no_vstar.hand.end(),
                                  sim::Card::RegidragoVstar),
                      no_vstar.hand.end());
  rejected(std::move(no_vstar), strict, 119702,
           "The route must require the held Regidrago VSTAR.");

  sim::State no_fire = route_state();
  no_fire.bench.front().fire = 0;
  rejected(std::move(no_fire), strict, 119703,
           "A target still missing Fire must remain outside the exact GF route.");

  sim::State no_serena = route_state();
  no_serena.hand.erase(std::remove(no_serena.hand.begin(), no_serena.hand.end(),
                                   sim::Card::Serena),
                       no_serena.hand.end());
  rejected(std::move(no_serena), strict, 119704,
           "The narrow route must preserve the held Serena continuation.");

  sim::State no_tapu = route_state();
  no_tapu.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  rejected(std::move(no_tapu), strict, 119705,
           "The narrow fix must not change unrelated Active-position states.");

  sim::State supporter_used = route_state();
  supporter_used.supporter_used = true;
  rejected(std::move(supporter_used), strict, 119706,
           "A spent Supporter slot must reject Tate's switch route.");

  const sim::Scenario no_control{"issue-1197-no-control",
                                 sim::DciProfile::NoDiscardControl,
                                 sim::LockMode::None, false, 5};
  rejected(route_state(), no_control, 119707,
           "No-discard-control must retain its existing payload-banking policy.");
}

void test_seed_11_reaches_turn_three_in_order() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-second scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{11};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const std::size_t hold = trace_index(
      trace, 2, "HOLD SUPPORTER", "held Regidrago VSTAR evolves");
  const std::size_t evolve = trace_index(
      trace, 2, "EVOLVE", "Regidrago V into Regidrago VSTAR");
  const std::size_t tate = trace_index(
      trace, 2, "PLAY SUPPORTER", "Tate & Liza switch mode");
  const std::size_t ready = trace_index(
      trace, 3, "READY", "Active Regidrago VSTAR has GGF");

  // The seeded regression proves the legal public ordering and the same-hidden-order
  // T3 result requested by the confirmed report:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1197
  expect(outcome.first_ready_turn == 3,
         "Seed 11 did not reach strict-JIT readiness on T3.");
  expect(hold < evolve && evolve < tate && tate < ready,
         "Seed 11 did not preserve evolve-then-switch ordering.");
}

}  // namespace

int main() {
  test_exact_evolve_then_switch_order();
  test_route_boundaries();
  test_seed_11_reaches_turn_three_in_order();
  return 0;
}
