#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"arven-fss-rulebox", sim::DciProfile::StrictJit,
                         sim::LockMode::FullRuleBoxAbility, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{211};
  sim::Engine engine{scenario, recipe, rng};
};

}  // namespace

int main() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::Arven};
  state.deck = {sim::Card::ForestSealStone, sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Dragapult, sim::Card::GoodraVstar,
                  sim::Card::DialgaGX, sim::Card::Dipplin, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Heavy Ball legally establishes the K1 state used to prove that the Tool is
  // the only live Arven target: https://api.pokemontcg.io/v2/cards/swsh10-146
  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before Arven is evaluated.");
  }

  // Forest Seal Stone gives Star Alchemy from the attached Tool, while Path to
  // the Peak removes Abilities only from Rule Box Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/swsh6-148
  if (!sim::EngineTestAccess::play_arven(fixture.engine)) {
    throw std::runtime_error("Arven should fetch the live Forest Seal Stone route through Rule Box lock.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::ForestSealStone) ||
      !contains(after.discard, sim::Card::Arven)) {
    throw std::runtime_error("Arven should take Forest Seal Stone and consume the Supporter slot.");
  }

  return 0;
}
