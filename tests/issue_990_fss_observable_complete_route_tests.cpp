#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static void set_state_k0(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool should_hold(const Engine& engine) {
    return engine.fss_should_hold_for_observable_complete_route();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void expect_trace_holds_star_alchemy(const sim::Scenario& scenario,
                                     const std::uint64_t seed,
                                     const int ready_turn) {
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();

  // Each exact current-main witness had a deterministic same-turn route from held
  // resources, yet Star Alchemy searched an unused card before readiness:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/990
  expect(outcome.first_ready_turn == ready_turn,
         "Holding Star Alchemy must preserve the earliest ready turn.");
  expect(!state.vstar_power_used,
         "The deterministic held route must preserve the VSTAR Power.");
}

sim::State complete_blender_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::Grass, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Crispin, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::GoodraVstar};
  return state;
}

bool should_hold(sim::State state) {
  const sim::Scenario scenario{"issue-990", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{990};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::should_hold(engine);
}

void test_four_full_trace_routes_hold_star_alchemy() {
  expect_trace_holds_star_alchemy(
      sim::Scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 4}, 180, 3);
  expect_trace_holds_star_alchemy(
      sim::Scenario{"strict-jit/go-second", sim::DciProfile::StrictJit,
                    sim::LockMode::None, false, 4}, 22, 2);
  // Issue #1130 restores the public Arven, Earthen Vessel, Dialga-GX, and manual
  // Grass route on T3 while the issue-990 policy still preserves Star Alchemy:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/issues/990
  // https://github.com/FlareZ123/pokemon-sims/issues/1130
  expect_trace_holds_star_alchemy(
      sim::Scenario{"strict-jit/go-second", sim::DciProfile::StrictJit,
                    sim::LockMode::None, false, 4}, 31, 3);
  expect_trace_holds_star_alchemy(
      sim::Scenario{"matchup-flex-jit/go-first", sim::DciProfile::MatchupFlexJit,
                    sim::LockMode::None, true, 5}, 85, 4);
}

void test_exact_complete_blender_state_holds() {
  expect(should_hold(complete_blender_state()),
         "Held Grass and Blender must complete without Star Alchemy.");
}

void test_missing_manual_energy_keeps_star_alchemy_live() {
  sim::State state = complete_blender_state();
  std::erase(state.hand, sim::Card::Grass);
  expect(!should_hold(std::move(state)),
         "Star Alchemy must remain live when Energy cannot complete.");
}

void test_spent_manual_attachment_keeps_star_alchemy_live() {
  sim::State state = complete_blender_state();
  state.manual_energy_used = true;
  expect(!should_hold(std::move(state)),
         "Star Alchemy must remain live after the manual attachment is spent.");
}

void test_missing_payload_outlet_keeps_star_alchemy_live() {
  sim::State state = complete_blender_state();
  std::erase(state.hand, sim::Card::BrilliantBlender);
  expect(!should_hold(std::move(state)),
         "Star Alchemy must remain live without a legal payload outlet.");
}

void test_seed_43_treasure_tapu_route_supersedes_star_alchemy() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{43};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();

  // The confirmed #1209 route establishes Regidrago V on T1, then uses held
  // Mysterious Treasure to discard Dragapult ex and search Tapu Lele-GX. Wonder Tag
  // obtains Crispin, reaching readiness on T2 while preserving the VSTAR Power:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(outcome.first_ready_turn == 2,
         "The Treasure-Tapu-Crispin route must reach readiness on turn two.");
  expect(!engine.state().vstar_power_used,
         "The earlier deterministic route must preserve the VSTAR Power.");
}

void test_k0_never_projects_hidden_complete_route() {
  const sim::Scenario scenario{"issue-990-k0", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{991};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = complete_blender_state();
  sim::EngineTestAccess::set_state_k0(engine, std::move(state));
  expect(!sim::EngineTestAccess::should_hold(engine),
         "K0 must not infer a hidden deterministic route.");
}

}  // namespace

int main() {
  try {
    test_four_full_trace_routes_hold_star_alchemy();
    test_exact_complete_blender_state_holds();
    test_missing_manual_energy_keeps_star_alchemy_live();
    test_spent_manual_attachment_keeps_star_alchemy_live();
    test_missing_payload_outlet_keeps_star_alchemy_live();
    test_seed_43_treasure_tapu_route_supersedes_star_alchemy();
    test_k0_never_projects_hidden_complete_route();
    std::cout << "Issue 990 FSS complete-route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
