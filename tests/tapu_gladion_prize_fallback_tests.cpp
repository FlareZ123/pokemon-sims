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
  static bool needs_tapu_connector(const Engine& engine) { return engine.needs_tapu_connector(); }
  static bool bench_tapu_if_useful(Engine& engine) { return engine.bench_tapu_if_useful(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const std::string& label, const std::uint64_t seed)
      : scenario{label, sim::DciProfile::NoDiscardControl, sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario, recipe, rng) {}
};

void test_quick_ball_fallback_uses_tapu_for_prized_regidrago_v() {
  Fixture fixture{"quick-k1-prized-regi", 6201};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::Dipplin};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX, sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Quick Ball discards another card, then searches the deck for a Basic Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  if (!sim::EngineTestAccess::play_quick_ball(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Quick Ball should take Tapu Lele-GX after proving Regidrago V is prized.");
  }

  // Wonder Tag searches a Supporter when Tapu Lele-GX is played from hand to Bench.
  // Gladion then exchanges for a revealed Prize card:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm4-95
  if (!sim::EngineTestAccess::bench_from_hand(fixture.engine, sim::Card::TapuLeleGX) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::Gladion) ||
      !sim::EngineTestAccess::play_gladion(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Tapu Lele-GX should chain Quick Ball into Gladion recovery of prized Regidrago V.");
  }
}

void test_mysterious_treasure_fallback_uses_tapu_for_prized_vstar() {
  Fixture fixture{"treasure-k1-prized-vstar", 6202};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX, sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Mysterious Treasure discards a hand card, then searches for a Psychic or Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Mysterious Treasure should take Tapu Lele-GX after proving Regidrago VSTAR is prized.");
  }

  // Wonder Tag and Gladion form a legal same-turn recovery line for the known Prize:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm4-95
  if (!sim::EngineTestAccess::bench_from_hand(fixture.engine, sim::Card::TapuLeleGX) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::Gladion) ||
      !sim::EngineTestAccess::play_gladion(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Tapu Lele-GX should chain Mysterious Treasure into Gladion recovery of prized Regidrago VSTAR.");
  }
}

void test_wonder_tag_fetches_tate_for_the_only_missing_active_axis() {
  Fixture fixture{"wonder-tag-tate-switch", 6203};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::Serena, sim::Card::TateLiza};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Wonder Tag can search Tate & Liza, then its switch mode can promote the powered
  // Benched Regidrago VSTAR using the still-open Supporter play:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!sim::EngineTestAccess::needs_tapu_connector(fixture.engine) ||
      !sim::EngineTestAccess::bench_tapu_if_useful(fixture.engine)) {
    throw std::runtime_error("Tapu Lele-GX should be a live connector for the only missing Active-position axis.");
  }

  const sim::State& after_wonder_tag = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after_wonder_tag.hand, sim::Card::TateLiza) ||
      contains(after_wonder_tag.hand, sim::Card::Serena)) {
    throw std::runtime_error("Wonder Tag should prefer Tate & Liza over draw-only Serena when switching completes readiness.");
  }
  if (!sim::EngineTestAccess::play_tate_switch(fixture.engine) ||
      !sim::EngineTestAccess::state(fixture.engine).active ||
      sim::EngineTestAccess::state(fixture.engine).active->card != sim::Card::RegidragoVstar) {
    throw std::runtime_error("Tate & Liza should promote the powered Benched Regidrago VSTAR.");
  }
}

}  // namespace

int main() {
  try {
    test_quick_ball_fallback_uses_tapu_for_prized_regidrago_v();
    test_mysterious_treasure_fallback_uses_tapu_for_prized_vstar();
    test_wonder_tag_fetches_tate_for_the_only_missing_active_axis();
    std::cout << "K1 Tapu connector tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
