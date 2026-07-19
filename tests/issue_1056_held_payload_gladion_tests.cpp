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

  static bool held_route_completes(const Engine& engine) {
    return engine.held_payload_completes_current_turn_without_gladion();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void expect_seed_holds_gladion(const char* scenario_label,
                               const std::uint64_t seed,
                               const int ready_turn,
                               const char* held_payload_cost) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("Missing scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  const bool spent_gladion = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("PLAY SUPPORTER") != std::string::npos &&
               line.find("R-GLADION-01") != std::string::npos;
      });
  const bool held_supporter = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("HOLD SUPPORTER") != std::string::npos &&
               line.find("Retained Gladion") != std::string::npos;
      });
  const bool paid_with_held_payload = std::any_of(
      trace.lines.begin(), trace.lines.end(), [held_payload_cost](const std::string& line) {
        return line.find("DISCARD") != std::string::npos &&
               line.find(held_payload_cost) != std::string::npos;
      });

  // A held permitted Dragon can pay the exact legal one-discard Item and satisfy
  // ready-turn JIT. Prize recovery adds no axis and spends the Supporter resource:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1056
  expect(outcome.first_ready_turn == ready_turn,
         "Held-payload route changed the earliest ready turn");
  expect(!spent_gladion, "Gladion was still spent for a redundant prized payload");
  expect(held_supporter, "The redundant Gladion route was not explicitly held");
  expect(paid_with_held_payload, "The live Item did not spend the held payload");
}

sim::State exact_vessel_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Dragapult,
                sim::Card::RegidragoVstar, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoV};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Crispin,
                  sim::Card::Arven, sim::Card::LatiasEx,
                  sim::Card::Channeler, sim::Card::Fire};
  return state;
}

bool held_route_completes(sim::State state) {
  const sim::Scenario scenario{"issue-1056", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1056};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_known_state(engine, std::move(state));
  return sim::EngineTestAccess::held_route_completes(engine);
}

void test_negative_boundaries() {
  expect(held_route_completes(exact_vessel_state()),
         "Exact held-payload Vessel route should complete without Gladion");

  sim::State spent_attachment = exact_vessel_state();
  spent_attachment.manual_energy_used = true;
  expect(!held_route_completes(std::move(spent_attachment)),
         "Gladion must remain available when the manual attachment is spent");

  sim::State no_legal_target = exact_vessel_state();
  no_legal_target.deck = {sim::Card::Arven, sim::Card::Crispin};
  expect(!held_route_completes(std::move(no_legal_target)),
         "Gladion must remain available when the Item has no legal target");
}

}  // namespace

int main() {
  expect_seed_holds_gladion("strict-jit/go-first", 42, 3,
                            "Dragapult ex (Earthen Vessel cost)");
  expect_seed_holds_gladion("strict-jit/go-first", 121, 3,
                            "Dialga-GX (Quick Ball cost)");
  expect_seed_holds_gladion("matchup-flex-jit/go-first", 77, 4,
                            "Dragapult ex (Quick Ball cost)");
  test_negative_boundaries();
  return 0;
}
