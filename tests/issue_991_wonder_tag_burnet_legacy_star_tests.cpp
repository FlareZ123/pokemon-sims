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
  static bool burnet_is_redundant(const Engine& engine) {
    return engine.wonder_tag_burnet_is_redundant_with_legacy_star_route();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-991", sim::DciProfile::StrictJit,
                                      sim::LockMode::None, true, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{991};
  return sim::Engine(scenario, recipe, rng);
}

sim::State complete_legacy_star_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::RegidragoVstar, sim::Card::FieldBlower};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell, sim::Card::TeamYellsCheer,
                sim::Card::MysteriousTreasure, sim::Card::Serena,
                sim::Card::Gladion};
  return state;
}

void test_legacy_star_complete_route_suppresses_burnet() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, complete_legacy_star_state());

  // Held Crispin plus the manual attachment completes GGF. The prior-turn
  // Regidrago V may evolve, then Legacy Star can establish the payload axis:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/991
  expect(sim::EngineTestAccess::burnet_is_redundant(engine),
         "The observable Legacy Star route must make Burnet redundant.");
}

void test_missing_crispin_keeps_burnet_live() {
  sim::Engine engine = make_engine();
  sim::State state = complete_legacy_star_state();
  std::erase(state.hand, sim::Card::Crispin);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::burnet_is_redundant(engine),
         "Burnet must remain live when the Energy route is incomplete.");
}

void test_used_vstar_power_keeps_burnet_live() {
  sim::Engine engine = make_engine();
  sim::State state = complete_legacy_star_state();
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::burnet_is_redundant(engine),
         "Burnet must remain live after the VSTAR Power is spent.");
}

void test_fresh_regidrago_keeps_burnet_live() {
  sim::Engine engine = make_engine();
  sim::State state = complete_legacy_star_state();
  state.active->entered_turn = state.turn;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::burnet_is_redundant(engine),
         "Burnet must remain live when Regidrago V cannot evolve this turn.");
}

void test_full_trace_skips_second_wonder_tag() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{136};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();
  const int tapu_in_play = static_cast<int>(std::count_if(
      state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
        return pokemon.card == sim::Card::TapuLeleGX;
      }));

  // Exact current-main witness from the confirmed issue:
  // https://github.com/FlareZ123/pokemon-sims/issues/991
  expect(outcome.first_ready_turn == 2,
         "The corrected route must preserve turn-two readiness.");
  expect(tapu_in_play == 1,
         "The redundant second Tapu Lele-GX must remain out of play.");
  expect(std::find(state.hand.begin(), state.hand.end(),
                   sim::Card::ProfessorBurnet) == state.hand.end(),
         "Unused Professor Burnet must not be searched into hand.");
}

}  // namespace

int main() {
  try {
    test_legacy_star_complete_route_suppresses_burnet();
    test_missing_crispin_keeps_burnet_live();
    test_used_vstar_power_keeps_burnet_live();
    test_fresh_regidrago_keeps_burnet_live();
    test_full_trace_skips_second_wonder_tag();
    std::cout << "Issue 991 Wonder Tag Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
