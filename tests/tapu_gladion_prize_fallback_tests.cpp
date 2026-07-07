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
  static bool play_mysterious_treasure(Engine& engine) { return engine.play_mysterious_treasure(false); }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
  static bool bench_from_hand(Engine& engine, const Card card) { return engine.bench_from_hand(card, true); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine(const std::string& label, std::mt19937_64& rng) {
  const sim::Scenario scenario{label, sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  return sim::Engine(scenario, sim::baseline_recipe(), rng);
}

void test_quick_ball_fallback_uses_tapu_for_prized_regidrago_v() {
  std::mt19937_64 rng{6201};
  sim::Engine engine = make_engine("quick-k1-prized-regi", rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::Dipplin};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX, sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball discards another card, then searches the deck for a Basic Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  if (!sim::EngineTestAccess::play_quick_ball(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Quick Ball should take Tapu Lele-GX after proving Regidrago V is prized.");
  }

  // Wonder Tag searches a Supporter when Tapu Lele-GX is played from hand to Bench.
  // Gladion then exchanges for a revealed Prize card:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm4-95
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Gladion) ||
      !sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Tapu Lele-GX should chain Quick Ball into Gladion recovery of prized Regidrago V.");
  }
}

void test_mysterious_treasure_fallback_uses_tapu_for_prized_vstar() {
  std::mt19937_64 rng{6202};
  sim::Engine engine = make_engine("treasure-k1-prized-vstar", rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX, sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Mysterious Treasure discards a hand card, then searches for a Psychic or Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (!sim::EngineTestAccess::play_mysterious_treasure(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Mysterious Treasure should take Tapu Lele-GX after proving Regidrago VSTAR is prized.");
  }

  // Wonder Tag and Gladion form a legal same-turn recovery line for the known Prize:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm4-95
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Gladion) ||
      !sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Tapu Lele-GX should chain Mysterious Treasure into Gladion recovery of prized Regidrago VSTAR.");
  }
}

}  // namespace

int main() {
  try {
    test_quick_ball_fallback_uses_tapu_for_prized_regidrago_v();
    test_mysterious_treasure_fallback_uses_tapu_for_prized_vstar();
    std::cout << "K1 Tapu Gladion fallback tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}