#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_turo_active_promotion_route(Engine& engine) {
    return engine.play_turo_active_promotion_route();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State turo_state(const int entered_turn, const int grass, const int fire,
                      const bool vstar_in_hand) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, entered_turn, grass, fire,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::ProfessorTuro};
  if (vstar_in_hand) state.hand.push_back(sim::Card::RegidragoVstar);
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void expect_turo_promoted(sim::Engine& engine, const char* route_message) {
  expect(sim::EngineTestAccess::play_turo_active_promotion_route(engine), route_message);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Professor Turo must promote the complete Benched Regidrago VSTAR.");
  expect(after.bench.empty() && contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.discard, sim::Card::ProfessorTuro),
         "The Basic Active must return to hand and Professor Turo must enter discard.");
  expect(after.supporter_used && !after.retreat_used,
         "The route must consume the Supporter play and preserve the retreat.");
}

void test_turo_promotes_when_prior_turn_active_has_no_vstar_in_hand() {
  const sim::Scenario scenario{"turo-prior-turn-no-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{399};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, turo_state(1, 2, 1, false));

  // Evolution requires the matching Evolution card from hand. Without Regidrago
  // VSTAR in hand, the prior-turn Active Regidrago V has no direct evolution route:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo may return that Basic and promote the complete Benched VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  expect_turo_promoted(engine,
      "Professor Turo should promote the ready Benched VSTAR when no Evolution card is in hand.");
}

void test_turo_promotes_when_prior_turn_active_lacks_ggf() {
  const sim::Scenario scenario{"turo-prior-turn-incomplete-active", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{400};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, turo_state(1, 1, 1, true));

  // Regidrago VSTAR's Apex Dragon requires two Grass and one Fire Energy. Evolving
  // this Active would leave it short of GGF, while Professor Turo can promote the
  // already complete Benched VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-171
  expect_turo_promoted(engine,
      "Professor Turo should promote the complete Benched VSTAR when the direct evolution lacks GGF.");
}

void test_turo_holds_for_ready_prior_turn_direct_evolution() {
  const sim::Scenario scenario{"turo-ready-prior-turn-evolution", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{401};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, turo_state(1, 2, 1, true));

  // A prior-turn Regidrago V with Regidrago VSTAR in hand and GGF already attached
  // has a ready direct evolution route, so Professor Turo should remain available:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Professor Turo should remain held for an equally ready direct evolution.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoV &&
             after.active->grass == 2 && after.active->fire == 1 &&
             after.bench.size() == 1U && contains(after.hand, sim::Card::ProfessorTuro) &&
             contains(after.hand, sim::Card::RegidragoVstar) && !after.supporter_used,
         "Holding Professor Turo must preserve the complete direct evolution state.");
}

void test_turo_promotes_behind_same_turn_active_regidrago() {
  const sim::Scenario scenario{"turo-same-turn-active-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{402};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, turo_state(3, 2, 1, true));

  // A Pokémon cannot evolve during the turn it entered play, so Professor Turo may
  // still promote the complete Benched VSTAR in this previously fixed state:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv4-171
  expect_turo_promoted(engine,
      "Professor Turo should preserve the same-turn Active Regidrago promotion route.");
}

}  // namespace

int main() {
  try {
    test_turo_promotes_when_prior_turn_active_has_no_vstar_in_hand();
    test_turo_promotes_when_prior_turn_active_lacks_ggf();
    test_turo_holds_for_ready_prior_turn_direct_evolution();
    test_turo_promotes_behind_same_turn_active_regidrago();
    std::cout << "Turo ready direct evolution tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
