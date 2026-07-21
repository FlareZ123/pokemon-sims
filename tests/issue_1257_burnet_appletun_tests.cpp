#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_professor_burnet(Engine& engine) {
    return engine.play_professor_burnet();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_appletun_payload() {
  const sim::Scenario scenario{"issue-1257", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1257};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Appletun, 1);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Appletun, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Professor Burnet searches the deck for up to two cards and discards them.
  // Appletun is a Dragon payload for Regidrago VSTAR's Apex Dragon:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1257
  if (!sim::EngineTestAccess::play_professor_burnet(engine)) {
    throw std::runtime_error("Professor Burnet did not play.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (contains(after.deck, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::ProfessorBurnet) ||
      !after.supporter_used) {
    throw std::runtime_error("Professor Burnet failed the Appletun payload route.");
  }
}

}  // namespace

int main() {
  try {
    test_appletun_payload();
    std::cout << "Issue 1257 Professor Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
