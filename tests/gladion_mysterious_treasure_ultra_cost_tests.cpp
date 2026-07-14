#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_gladion_holds_for_mysterious_treasure_paid_by_ultra_ball() {
  // Mysterious Treasure discards one other card before searching for a Psychic or
  // Dragon Pokémon, so a distinct Ultra Ball is a legal cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // Ultra Ball's own two-other-card cost does not control whether it may be discarded
  // by Mysterious Treasure: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Gladion should remain unused while the direct deck-search route is payable:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-mysterious-ultra-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{359};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::MysteriousTreasure, sim::Card::UltraBall};
  state.deck = {sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma, sim::Card::Oricorio};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be held while Mysterious Treasure can discard Ultra Ball and find Regidrago VSTAR");
  }
  const sim::State& held = sim::EngineTestAccess::state(engine);
  if (held.supporter_used || !contains(held.hand, sim::Card::Gladion) ||
      !contains(held.hand, sim::Card::MysteriousTreasure) ||
      !contains(held.hand, sim::Card::UltraBall)) {
    throw std::runtime_error("The payable direct VSTAR route must preserve Gladion and the exact hand before Item play");
  }

  if (!sim::EngineTestAccess::play_mysterious_treasure(engine)) {
    throw std::runtime_error("Mysterious Treasure must accept the distinct Ultra Ball as its discard cost");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.supporter_used || !contains(after.hand, sim::Card::Gladion) ||
      !contains(after.hand, sim::Card::RegidragoVstar) ||
      contains(after.hand, sim::Card::MysteriousTreasure) ||
      contains(after.hand, sim::Card::UltraBall) ||
      !contains(after.discard, sim::Card::MysteriousTreasure) ||
      !contains(after.discard, sim::Card::UltraBall) || !after.deck.empty()) {
    throw std::runtime_error("Mysterious Treasure must discard Ultra Ball, find Regidrago VSTAR, and leave Gladion unused");
  }
}

}  // namespace

int main() {
  test_gladion_holds_for_mysterious_treasure_paid_by_ultra_ball();
  return 0;
}
