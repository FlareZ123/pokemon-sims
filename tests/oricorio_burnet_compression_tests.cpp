#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

void test_oricorio_preserves_burnet_for_same_turn_readiness() {
  const sim::Scenario scenario{"oricorio-burnet-compression", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{4242};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Oricorio, sim::Card::Crispin, sim::Card::ProfessorBurnet,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::MegaDragonite};
  // Vital Dance searches the final Basic Fire Energy when Oricorio is played from
  // hand to the Bench: https://api.pokemontcg.io/v2/cards/sm2-55
  // Professor Burnet then searches the deck and discards the Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V entered play on turn 1, so it may evolve on turn 2 under the core
  // evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::set_state(engine, std::move(state));

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!benched(after, sim::Card::Oricorio)) {
    throw std::runtime_error("Oricorio should be benched to preserve Professor Burnet's Supporter turn.");
  }
  if (!after.active || after.active->card != sim::Card::RegidragoVstar || after.active->grass != 2 || after.active->fire != 1) {
    throw std::runtime_error("The Oricorio/manual Energy line should evolve and complete GGF this turn.");
  }
  if (!contains(after.discard, sim::Card::ProfessorBurnet) || !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Professor Burnet should use the preserved Supporter play to discard Mega Dragonite ex.");
  }
  if (!contains(after.hand, sim::Card::Crispin)) {
    throw std::runtime_error("Crispin should remain available after Oricorio supplies the Energy axis.");
  }
  if (!sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The Burnet discard must satisfy strict current-turn payload timing.");
  }
}

}  // namespace

int main() {
  test_oricorio_preserves_burnet_for_same_turn_readiness();
  return 0;
}
