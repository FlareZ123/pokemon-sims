#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
  static bool bench_from_hand(Engine& engine, Card card, bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static bool play_mysterious_treasure(Engine& engine, bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_mawile_starts_active_so_oricorio_keeps_vital_dance() {
  // Vital Dance triggers only when Oricorio is played from hand onto the Bench
  // during the turn. Mawile-GX is a Basic Pokémon and can legally take the opening
  // Active Spot while Oricorio remains in hand:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"oricorio-opening-preservation", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{306};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::MawileGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::MawileGX,
         "Mawile-GX should take the opening Active Spot when it is paired with Oricorio");
  expect(contains(after.hand, sim::Card::Oricorio),
         "Oricorio should remain in hand so Vital Dance can trigger when it is Benched during the turn");
}

void test_oricorio_selects_a_second_legal_energy_for_follow_up_search() {
  // Vital Dance may search up to 2 Basic Energy cards: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure requires a hand-discard cost: https://api.pokemontcg.io/v2/cards/sm6-113
  sim::Scenario scenario{"oricorio-full-energy-search", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Oricorio, sim::Card::Fire, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Fire, sim::Card::Grass};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{97};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::bench_from_hand(engine, sim::Card::Oricorio, true),
         "Vital Dance should resolve when Oricorio is legally Benched from hand.");
  const sim::State& after_vital_dance = sim::EngineTestAccess::state(engine);
  expect(count(after_vital_dance.hand, sim::Card::Fire) == 2,
         "Vital Dance should keep the second legal Fire Energy after finding the needed Grass Energy.");
  expect(contains(after_vital_dance.hand, sim::Card::Grass),
         "Vital Dance should still find the Grass Energy that advances GGF.");

  expect(sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "The second Fire Energy should make Mysterious Treasure's discard cost payable.");
  const sim::State& after_treasure = sim::EngineTestAccess::state(engine);
  expect(contains(after_treasure.hand, sim::Card::RegidragoVstar),
         "Mysterious Treasure should search Regidrago VSTAR after the legal Fire discard.");
}

}  // namespace

int main() {
  try {
    test_mawile_starts_active_so_oricorio_keeps_vital_dance();
    test_oricorio_selects_a_second_legal_energy_for_follow_up_search();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
