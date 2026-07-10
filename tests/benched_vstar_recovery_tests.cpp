#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State benched_regidrago_recovery_state(const sim::Card supporter) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None}};
  state.hand = {supporter, sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void assert_completed_benched_recovery(const sim::State& state, const sim::Card supporter) {
  assert(state.supporter_used);
  assert(state.retreat_used);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.active->grass == 2);
  assert(state.active->fire == 1);
  assert(contains(state.discard, supporter));
  assert(contains(state.discard, sim::Card::EvolutionIncense));
  assert(!contains(state.discard, sim::Card::RegidragoVstar));
  assert(contains(state.discarded_this_turn, sim::Card::MegaDragonite));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
    return pokemon.card == sim::Card::LatiasEx;
  }));
}

void test_roseanne_restores_benched_vstar_evolution_and_promotion() {
  const sim::Scenario scenario{"roseanne-benched-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3111};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::state(engine) = benched_regidrago_recovery_state(sim::Card::RoseannesBackup);
  sim::EngineTestAccess::set_deck_seen(engine);

  // Roseanne's Backup restores the discarded Pokémon, Evolution Incense searches
  // that Evolution, and a prior-turn Benched Regidrago V may evolve. Latias ex then
  // gives the Basic Active no Retreat Cost for promotion:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);
  assert_completed_benched_recovery(sim::EngineTestAccess::state(engine), sim::Card::RoseannesBackup);
}

void test_team_yell_restores_benched_vstar_evolution_and_promotion() {
  const sim::Scenario scenario{"team-yell-benched-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3112};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::state(engine) = benched_regidrago_recovery_state(sim::Card::TeamYellsCheer);
  sim::EngineTestAccess::set_deck_seen(engine);

  // Team Yell's Cheer restores the discarded Pokémon, Evolution Incense searches
  // that Evolution, and the legal evolution target may be Benched. Latias ex supplies
  // the free-retreat promotion after the evolution resolves:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);
  assert_completed_benched_recovery(sim::EngineTestAccess::state(engine), sim::Card::TeamYellsCheer);
}

}  // namespace

int main() {
  test_roseanne_restores_benched_vstar_evolution_and_promotion();
  test_team_yell_restores_benched_vstar_evolution_and_promotion();
  return 0;
}
