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
  static bool arven_is_redundant(const Engine& engine) {
    return engine.wonder_tag_arven_is_redundant_with_held_burnet_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-993", sim::DciProfile::MatchupFlexJit,
                                      sim::LockMode::None, false, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{993};
  return sim::Engine(scenario, recipe, rng);
}

sim::State complete_burnet_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::ProfessorBurnet,
                sim::Card::Fire, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Arven, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass};
  return state;
}

void test_held_burnet_route_suppresses_arven() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, complete_burnet_state());
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/993
  expect(sim::EngineTestAccess::arven_is_redundant(engine),
         "Held Burnet, Fire, and VSTAR must make Arven redundant.");
}

void test_missing_burnet_preserves_arven() {
  sim::Engine engine = make_engine();
  sim::State state = complete_burnet_state();
  std::erase(state.hand, sim::Card::ProfessorBurnet);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::arven_is_redundant(engine),
         "Arven must remain live without the held payload bridge.");
}

void test_missing_vstar_preserves_arven() {
  sim::Engine engine = make_engine();
  sim::State state = complete_burnet_state();
  std::erase(state.hand, sim::Card::RegidragoVstar);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::arven_is_redundant(engine),
         "Arven must remain live when evolution is unresolved.");
}

void test_wrong_energy_preserves_arven() {
  sim::Engine engine = make_engine();
  sim::State state = complete_burnet_state();
  std::erase(state.hand, sim::Card::Fire);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::arven_is_redundant(engine),
         "Arven must remain live when the final Energy is absent.");
}

void test_full_trace_skips_unused_arven() {
  const sim::Scenario scenario{"matchup-flex-jit/go-second",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{143};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();
  expect(outcome.first_ready_turn == 2,
         "The corrected route must preserve turn-two readiness.");
  expect(std::none_of(state.bench.begin(), state.bench.end(),
                      [](const sim::Pokemon& pokemon) {
                        return pokemon.card == sim::Card::TapuLeleGX;
                      }),
         "The redundant Tapu Lele-GX must remain out of play.");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::Arven) ==
             state.hand.end(),
         "Unused Arven must not be searched into hand.");
}
}  // namespace

int main() {
  try {
    test_held_burnet_route_suppresses_arven();
    test_missing_burnet_preserves_arven();
    test_missing_vstar_preserves_arven();
    test_wrong_energy_preserves_arven();
    test_full_trace_skips_unused_arven();
    std::cout << "Issue 993 Wonder Tag Arven tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
