#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_serena(Engine& engine, const bool allow_payload = false) {
    return engine.play_serena(allow_payload);
  }
};

}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State locked_item_state(const int item_count) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Serena, sim::Card::Channeler, sim::Card::Dragapult};
  if (item_count >= 1) state.hand.push_back(sim::Card::EarthenVessel);
  if (item_count >= 2) state.hand.push_back(sim::Card::MysteriousTreasure);
  if (item_count >= 3) state.hand.push_back(sim::Card::QuickBall);
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::LatiasEx, sim::Card::Oricorio};
  return state;
}

void verify_full_lock_item_count(const sim::LockMode lock, const int item_count) {
  sim::Scenario scenario{"issue-1855-full-lock", sim::DciProfile::StrictJit,
                         lock, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185500U + static_cast<unsigned>(item_count)};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, locked_item_state(item_count));

  // Serena may discard one to three cards, then draws until the hand has five.
  // Full Item and combined lock leave these visible Items unusable for every
  // remaining modeled turn, making each one DCI 1:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Full Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena did not resolve under permanent Item lock");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  const int discarded_items = count(after.discard, sim::Card::EarthenVessel) +
      count(after.discard, sim::Card::MysteriousTreasure) +
      count(after.discard, sim::Card::QuickBall);
  expect(discarded_items == item_count,
         "Serena did not discard every draw-improving permanently locked Item");
  expect(contains(after.hand, sim::Card::Channeler),
         "Serena discarded a live Supporter before a dead Item");
  expect(contains(after.hand, sim::Card::Dragapult),
         "Serena discarded a protected strict-JIT payload");
  expect(after.hand.size() == 5U,
         "Serena did not draw to five after the projected discard plan");
}

void test_one_two_three_dead_items() {
  for (int item_count = 1; item_count <= 3; ++item_count) {
    verify_full_lock_item_count(sim::LockMode::FullItem, item_count);
  }
  verify_full_lock_item_count(sim::LockMode::FullCombined, 3);
}

void test_scheduled_lock_does_not_promote_currently_live_items() {
  sim::Scenario scenario{"issue-1855-scheduled-lock", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185504};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Serena, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                sim::Card::Channeler, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::LatiasEx};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // TurnTwoItem is still unlocked on turn one. The permanent-lock override must
  // remain inactive, leaving the singleton Earthen Vessel available for legal play:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Scheduled lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena did not resolve in the scheduled-lock control");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.discard, sim::Card::MysteriousTreasure) == 1,
         "The duplicate strict-DCI cost changed in the scheduled-lock control");
  expect(contains(after.hand, sim::Card::EarthenVessel),
         "A currently live Item was treated as permanently locked");
}

void test_zero_draw_payload_completion_preserves_dead_items() {
  sim::Scenario scenario{"issue-1855-zero-draw", sim::DciProfile::StrictJit,
                         sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185505};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Serena, sim::Card::MegaDragonite,
                sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::Channeler,
                sim::Card::Crispin, sim::Card::LatiasEx};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The dedicated strict-JIT completion branch must discard the Dragon payload.
  // With six cards left after Serena and that payload leave hand, discarding one
  // additional Item would add no draw, so every optional Item remains held:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Full Item-lock and DCI contracts: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine, true),
         "Serena did not complete the zero-draw strict-JIT payload route");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MegaDragonite),
         "The required Dragon payload was not discarded");
  expect(contains(after.hand, sim::Card::EarthenVessel) &&
             contains(after.hand, sim::Card::MysteriousTreasure) &&
             contains(after.hand, sim::Card::QuickBall),
         "Serena spent a dead Item without gaining a draw");
}

}  // namespace

int main() {
  try {
    test_one_two_three_dead_items();
    test_scheduled_lock_does_not_promote_currently_live_items();
    test_zero_draw_payload_completion_preserves_dead_items();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
