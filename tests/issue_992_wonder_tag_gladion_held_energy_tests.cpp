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
    engine.prizes_revealed_ = true;
  }
  static bool gladion_is_redundant(const Engine& engine) {
    return engine.wonder_tag_gladion_is_redundant_with_held_manual_energy();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-992", sim::DciProfile::NoDiscardControl,
                                      sim::LockMode::None, true, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{992};
  return sim::Engine(scenario, recipe, rng);
}

sim::State complete_manual_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Fire,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Gladion, sim::Card::Grass,
                sim::Card::RegidragoV};
  state.prizes = {sim::Card::Fire, sim::Card::Crispin,
                  sim::Card::FieldBlower, sim::Card::TeamYellsCheer,
                  sim::Card::DialgaGX, sim::Card::RegidragoV};
  state.discard = {sim::Card::MegaDragonite};
  return state;
}

void test_held_prize_target_energy_suppresses_gladion() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, complete_manual_state());
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Manual attachment: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/992
  expect(sim::EngineTestAccess::gladion_is_redundant(engine),
         "Held Fire plus the manual attachment must make Gladion redundant.");
}

void test_wrong_held_type_preserves_gladion() {
  sim::Engine engine = make_engine();
  sim::State state = complete_manual_state();
  std::erase(state.hand, sim::Card::Fire);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::gladion_is_redundant(engine),
         "Gladion must remain live when the Prize-target type is absent from hand.");
}

void test_used_manual_attachment_preserves_gladion() {
  sim::Engine engine = make_engine();
  sim::State state = complete_manual_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::gladion_is_redundant(engine),
         "Gladion must remain live after the manual attachment is spent.");
}

void test_missing_payload_preserves_gladion() {
  sim::Engine engine = make_engine();
  sim::State state = complete_manual_state();
  state.discard.clear();
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::gladion_is_redundant(engine),
         "Gladion must remain live while another setup axis is incomplete.");
}

void test_full_trace_skips_unused_gladion() {
  const sim::Scenario scenario{"no-discard-control/go-first",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{80};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();
  expect(outcome.first_ready_turn == 3,
         "The corrected route must preserve turn-three readiness.");
  expect(std::none_of(state.bench.begin(), state.bench.end(),
                      [](const sim::Pokemon& pokemon) {
                        return pokemon.card == sim::Card::TapuLeleGX;
                      }),
         "The redundant Tapu Lele-GX must remain out of play.");
  expect(std::find(state.hand.begin(), state.hand.end(), sim::Card::Gladion) ==
             state.hand.end(),
         "Unused Gladion must not be searched into hand.");
}
}  // namespace

int main() {
  try {
    test_held_prize_target_energy_suppresses_gladion();
    test_wrong_held_type_preserves_gladion();
    test_used_manual_attachment_preserves_gladion();
    test_missing_payload_preserves_gladion();
    test_full_trace_skips_unused_gladion();
    std::cout << "Issue 992 Wonder Tag Gladion tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
