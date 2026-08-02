#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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
  static bool tate_burnet_route(const Engine& engine) {
    return engine.tate_switch_then_next_turn_burnet_route();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1}};
  state.hand = {sim::Card::TateLiza, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

bool trace_contains(const sim::TraceLog& trace, const int turn,
                    const std::string& action, const std::string& detail) {
  const std::string prefix = "T" + std::to_string(turn) + " | ";
  for (const std::string& line : trace.lines) {
    if (line.starts_with(prefix) && line.find(action) != std::string::npos &&
        line.find(detail) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void test_t3_projection_remains_live() {
  const sim::Scenario scenario{"issue-2159-t3-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{215900};
  sim::Engine engine = make_engine(scenario, rng, route_state(3));

  // A T3 Tate & Liza switch can still preserve Professor Burnet for T4, which is
  // the final setup-success turn under the repository contract:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // One-Supporter-per-turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // T4 setup-success cutoff: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
  // Confirmed boundary bug: https://github.com/FlareZ123/pokemon-sims/issues/2159
  expect(sim::EngineTestAccess::tate_burnet_route(engine),
         "The valid T3 switch into T4 Burnet route was rejected.");
}

void test_t4_projection_is_rejected() {
  const sim::Scenario scenario{"issue-2159-t4-boundary",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{215901};
  sim::Engine engine = make_engine(scenario, rng, route_state(4));

  // T5 remains observable only for diagnostic recovery. A T4 Supporter must not be
  // spent on a projected Burnet continuation that can complete only on diagnostic T5:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // T5 diagnostic accounting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc
  // T4 setup-success cutoff: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
  // Confirmed boundary bug: https://github.com/FlareZ123/pokemon-sims/issues/2159
  expect(!sim::EngineTestAccess::tate_burnet_route(engine),
         "The T4 switch into diagnostic-T5 Burnet route remained selectable.");
}

void test_seed_2244_does_not_take_t4_tate_projection() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2244};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  engine.run();

  // The merged-source reproduction spent Tate & Liza on T4 to preserve Professor
  // Burnet for T5. The corrected policy must reject that expired projection. A
  // separate legal route may still produce diagnostic T5 readiness, which remains
  // observable under the repository contract:
  // Reproduction and exact public state: https://github.com/FlareZ123/pokemon-sims/issues/2159
  // Repository deadline: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  expect(!trace_contains(trace, 4, "PLAY SUPPORTER", "Tate & Liza switch mode"),
         "Seed 2244 still spent Tate & Liza on the expired T4 projection.");
}

}  // namespace

int main() {
  test_t3_projection_remains_live();
  test_t4_projection_is_rejected();
  test_seed_2244_does_not_take_t4_tate_projection();
  return 0;
}
