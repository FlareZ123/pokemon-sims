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
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool active_vstar_before_tate(const Engine& engine) {
    return engine.tate_draw_after_active_vstar_evolution_route();
  }

  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool evolve_best_regi(Engine& engine) { return engine.evolve_best_regi(); }
  static bool play_tate_draw(Engine& engine) { return engine.play_tate_draw(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::TateLiza, sim::Card::RegidragoVstar,
                sim::Card::Arven};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Guzma,
                sim::Card::LatiasEx, sim::Card::Serena,
                sim::Card::MysteriousTreasure, sim::Card::Grass};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::QuickBall,
                  sim::Card::Arven, sim::Card::EarthenVessel,
                  sim::Card::Crispin, sim::Card::Gladion};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_known_state(engine, std::move(state));
  return engine;
}

std::size_t trace_index(const sim::TraceLog& trace, const int turn,
                        const std::string& action,
                        const std::string& detail) {
  const std::string turn_prefix = "T" + std::to_string(turn) + " | ";
  for (std::size_t index = 0; index < trace.lines.size(); ++index) {
    const std::string& line = trace.lines[index];
    if (line.starts_with(turn_prefix) &&
        line.find(action) != std::string::npos &&
        line.find(detail) != std::string::npos) {
      return index;
    }
  }
  return trace.lines.size();
}

void test_exact_ordering_route() {
  const sim::Scenario scenario{"issue-1162", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, true, 5};
  std::mt19937_64 rng{116200};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario, rng, route_state(), &trace);

  // Tate & Liza draws five after shuffling the remaining hand. The prior-turn Active
  // Regidrago V may first evolve with the held Star Alchemy target, and its complete
  // GGF means Celestial Roar cannot advance strict-JIT setup in this state:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Evolution, Supporter, search, shuffle, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest legal route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1162
  expect(sim::EngineTestAccess::active_vstar_before_tate(engine),
         "The exact Active GGF Star Alchemy route was not recognized.");
  sim::EngineTestAccess::choose_supporter(engine);
  expect(!engine.state().supporter_used,
         "Tate draw mode ran before the searched evolution was used.");
  expect(sim::EngineTestAccess::evolve_best_regi(engine),
         "The prior-turn Active Regidrago V did not evolve.");
  expect(engine.state().active &&
             engine.state().active->card == sim::Card::RegidragoVstar,
         "The searched VSTAR did not remain established on the board.");
  expect(sim::EngineTestAccess::play_tate_draw(engine),
         "Tate draw mode did not resume after evolution.");
  expect(engine.state().supporter_used,
         "Tate draw mode did not consume the Supporter slot.");
  expect(engine.state().hand.size() == 5,
         "Tate draw mode did not draw exactly five cards.");
  expect(engine.state().active &&
             engine.state().active->card == sim::Card::RegidragoVstar,
         "Tate draw mode removed the evolved Active from play.");
}

void expect_route_rejected(sim::State state, const sim::Scenario& scenario,
                           const std::uint64_t seed, const char* message) {
  std::mt19937_64 rng{seed};
  sim::Engine engine = make_engine(scenario, rng, std::move(state));
  expect(!sim::EngineTestAccess::active_vstar_before_tate(engine), message);
}

void test_negative_boundaries() {
  const sim::Scenario locked{"issue-1162-controls", sim::DciProfile::StrictJit,
                             sim::LockMode::TurnTwoItem, true, 5};

  sim::State same_turn = route_state();
  same_turn.active->entered_turn = same_turn.turn;
  expect_route_rejected(std::move(same_turn), locked, 116201,
                        "Same-turn Regidrago V must not evolve.");

  sim::State energy_incomplete = route_state();
  energy_incomplete.active->grass = 1;
  expect_route_rejected(std::move(energy_incomplete), locked, 116202,
                        "Energy-incomplete Active must preserve productive Celestial Roar.");

  sim::State no_vstar = route_state();
  no_vstar.hand.erase(std::remove(no_vstar.hand.begin(), no_vstar.hand.end(),
                                  sim::Card::RegidragoVstar),
                      no_vstar.hand.end());
  expect_route_rejected(std::move(no_vstar), locked, 116203,
                        "Absent Regidrago VSTAR must not hold Tate.");

  const sim::Scenario no_lock{"issue-1162-no-lock", sim::DciProfile::StrictJit,
                              sim::LockMode::None, true, 5};
  expect_route_rejected(route_state(), no_lock, 116204,
                        "Ordinary non-lock states must retain the existing Tate policy.");

  const sim::Scenario no_discard_control{
      "issue-1162-ndc", sim::DciProfile::NoDiscardControl,
      sim::LockMode::TurnTwoItem, true, 5};
  expect_route_rejected(route_state(), no_discard_control, 116205,
                        "A profile that can bank payloads with Celestial Roar must remain outside this guard.");

  sim::State used_supporter = route_state();
  used_supporter.supporter_used = true;
  expect_route_rejected(std::move(used_supporter), locked, 116206,
                        "A spent Supporter slot must reject the Tate ordering guard.");
}

void test_seed_31_full_trace_order() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-turn2-item-lock/go-first");
  if (!scenario) throw std::runtime_error("Missing turn-two Item-lock scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{31};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  engine.run();

  const std::size_t search = trace_index(
      trace, 3, "STAR ALCHEMY", "Regidrago VSTAR");
  const std::size_t evolve = trace_index(
      trace, 3, "EVOLVE", "Regidrago V into Regidrago VSTAR");
  const std::size_t tate = trace_index(
      trace, 3, "PLAY SUPPORTER", "Tate & Liza draw mode");

  // The deterministic seed reproduces the exact reported public/K1 state. The legal
  // evolution must occur between the one-shot search and Tate's fixed-five refresh:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core evolution, search, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Full-trace regression: https://github.com/FlareZ123/pokemon-sims/issues/1162
  expect(search < trace.lines.size(),
         "Seed 31 did not search Regidrago VSTAR with Star Alchemy.");
  expect(evolve < trace.lines.size(),
         "Seed 31 did not evolve the Active Regidrago V on T3.");
  expect(tate < trace.lines.size(),
         "Seed 31 did not resolve Tate draw mode after evolution.");
  expect(search < evolve && evolve < tate,
         "Seed 31 retained the dominated Star Alchemy, Tate, evolution order.");
}

}  // namespace

int main() {
  test_exact_ordering_route();
  test_negative_boundaries();
  test_seed_31_full_trace_order();
  return 0;
}
