#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void setup(Engine& engine) { engine.setup(); }
  static void begin_turn(Engine& engine, const int turn) {
    engine.begin_turn(turn);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool tate_route_completes(const Engine& engine) {
    return engine.tate_draw_has_held_non_supporter_completion();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_has(const sim::TraceLog& trace, const int turn,
               const std::string& action, const std::string& detail) {
  const std::string turn_prefix = "T" + std::to_string(turn) + " | ";
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.starts_with(turn_prefix) &&
                              line.find(action) != std::string::npos &&
                              line.find(detail) != std::string::npos;
                     });
}

sim::State seed_31_state_before_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-JIT going-second scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{31};
  sim::Engine engine(*scenario, recipe, rng);
  sim::EngineTestAccess::setup(engine);
  sim::EngineTestAccess::begin_turn(engine, 1);
  sim::EngineTestAccess::run_turn(engine);
  sim::EngineTestAccess::begin_turn(engine, 2);
  sim::EngineTestAccess::run_turn(engine);
  sim::EngineTestAccess::begin_turn(engine, 3);

  return engine.state();
}

bool tate_route_completes(sim::State state, const sim::DciProfile dci) {
  const sim::Scenario scenario{"issue-1130", dci, sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1130};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_known_state(engine, std::move(state));
  return sim::EngineTestAccess::tate_route_completes(engine);
}

sim::State observable_blender_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::TateLiza, sim::Card::Grass,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::RegidragoV, sim::Card::Fire};
  state.prizes = {sim::Card::Dragapult, sim::Card::Crispin,
                  sim::Card::Arven, sim::Card::LatiasEx,
                  sim::Card::Channeler, sim::Card::GoodraVstar};
  return state;
}

void test_private_legacy_star_projection_is_rejected() {
  // The seed-31 public state holds Arven, Earthen Vessel's Dialga-GX cost, and the
  // final manual Grass route. A favorable private Legacy Star top seven cannot make
  // Tate's hold deterministic:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Hidden-order policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1130
  expect(!tate_route_completes(seed_31_state_before_t3(),
                               sim::DciProfile::StrictJit),
         "Private Legacy Star order still classified seed 31 as a held route");
}

void test_observable_held_route_is_preserved() {
  // Held Grass plus Brilliant Blender proves completion from visible resources and
  // must continue to suppress Tate's random hand replacement:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Preserved behavior: https://github.com/FlareZ123/pokemon-sims/issues/1057
  expect(tate_route_completes(observable_blender_state(),
                              sim::DciProfile::MatchupFlexJit),
         "Observable Grass plus Blender route no longer holds Tate");
}

void test_seed_31_uses_public_t3_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-JIT going-second scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{31};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Arven legally searches Earthen Vessel and Forest Seal Stone. Vessel discards
  // Dialga-GX, searches Grass, and the manual attachment completes GGF on T3:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, Item, Tool, discard, and Energy procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1130
  expect(outcome.first_ready_turn == 3,
         "Seed 31 did not reach readiness through the public T3 route");
  expect(!trace_has(trace, 3, "HOLD SUPPORTER", "Retained Tate & Liza"),
         "Seed 31 still holds Tate on private Legacy Star information");
  expect(trace_has(trace, 3, "PLAY SUPPORTER", "R-ARVEN-01"),
         "Seed 31 did not play Arven on T3");
  expect(trace_has(trace, 3, "DISCARD", "Dialga-GX (Earthen Vessel cost)"),
         "Seed 31 did not use Dialga-GX as the T3 Vessel payload cost");
  expect(trace_has(trace, 3, "READY", "permitted Dragon payload"),
         "Seed 31 trace did not record T3 readiness");
  expect(!outcome.used_fss,
         "Seed 31 unnecessarily spent Star Alchemy despite the held route");
}

}  // namespace

int main() {
  test_private_legacy_star_projection_is_rejected();
  test_observable_held_route_is_preserved();
  test_seed_31_uses_public_t3_route();
  return 0;
}
