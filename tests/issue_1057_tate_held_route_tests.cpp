#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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

  static bool tate_route_completes(const Engine& engine) {
    return engine.tate_draw_has_held_non_supporter_completion();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void expect_seed_holds_tate(const char* scenario_label,
                            const std::uint64_t seed,
                            const int ready_turn,
                            const char* required_action) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("Missing scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  const bool played_tate = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("PLAY SUPPORTER") != std::string::npos &&
               line.find("R-TATE-01") != std::string::npos;
      });
  const bool held_tate = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("HOLD SUPPORTER") != std::string::npos &&
               line.find("Retained Tate & Liza") != std::string::npos;
      });
  const bool used_held_route = std::any_of(
      trace.lines.begin(), trace.lines.end(), [required_action](const std::string& line) {
        return line.find(required_action) != std::string::npos;
      });

  // Tate & Liza draw mode shuffles the whole hand before drawing five. When held
  // non-Supporter cards already complete VSTAR, GGF, and ready-turn payload, the
  // deterministic legal route must resolve before that random hand replacement:
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1057
  expect(outcome.first_ready_turn == ready_turn,
         "Holding Tate changed the expected earliest ready turn");
  expect(!played_tate, "Tate & Liza still shuffled away the held complete route");
  expect(held_tate, "The complete held route was not classified before Tate draw");
  expect(used_held_route, "The expected held non-Supporter route was not used");
}

sim::State exact_blender_state() {
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

bool tate_route_completes(sim::State state) {
  const sim::Scenario scenario{"issue-1057", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1057};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_known_state(engine, std::move(state));
  return sim::EngineTestAccess::tate_route_completes(engine);
}

void test_negative_boundaries() {
  expect(tate_route_completes(exact_blender_state()),
         "Exact Grass plus Blender route should hold Tate");

  sim::State spent_attachment = exact_blender_state();
  spent_attachment.manual_energy_used = true;
  expect(!tate_route_completes(std::move(spent_attachment)),
         "Tate must remain available when the manual attachment is spent");

  sim::State no_payload = exact_blender_state();
  no_payload.deck = {sim::Card::RegidragoV, sim::Card::Grass,
                     sim::Card::Fire};
  expect(!tate_route_completes(std::move(no_payload)),
         "Tate must remain available when Blender has no permitted payload");
}

}  // namespace

int main() {
  expect_seed_holds_tate("matchup-flex-jit/go-first", 185, 3,
                         "Searched and discarded: Mega Dragonite ex");
  expect_seed_holds_tate("strict-jit/go-second", 13, 2,
                         "Mega Dragonite ex (Mysterious Treasure cost)");
  expect_seed_holds_tate("strict-jit-rulebox-ability-lock/go-second", 13, 2,
                         "Mega Dragonite ex (Mysterious Treasure cost)");
  test_negative_boundaries();
  return 0;
}
