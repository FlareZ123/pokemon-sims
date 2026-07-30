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

sim::State exact_three_item_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Serena, sim::Card::Channeler, sim::Card::Dragapult,
                sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::LatiasEx, sim::Card::Oricorio};
  return state;
}

void verify_exact_three_item_refresh(const sim::LockMode lock) {
  sim::Scenario scenario{"issue-1855-full-lock", sim::DciProfile::StrictJit,
                         lock, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185500};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_three_item_state());

  // Preserve the already-productive Serena decision, then use both optional
  // discard slots on Items that can never be played in this scenario:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Full Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena did not resolve in the confirmed productive state");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.discard, sim::Card::EarthenVessel) == 1,
         "Serena retained a permanently locked Earthen Vessel");
  expect(count(after.discard, sim::Card::MysteriousTreasure) == 2,
         "Serena retained a permanently locked Mysterious Treasure");
  expect(contains(after.hand, sim::Card::Channeler),
         "Serena discarded a live Supporter before a dead Item");
  expect(contains(after.hand, sim::Card::Dragapult),
         "Serena discarded a protected strict-JIT payload");
  expect(after.hand.size() == 5U,
         "Serena did not draw to five after three legal discards");
}

void test_full_and_combined_lock_refresh() {
  verify_exact_three_item_refresh(sim::LockMode::FullItem);
  verify_exact_three_item_refresh(sim::LockMode::FullCombined);
}

void test_one_locked_item_uses_optional_slot_after_existing_first_cost() {
  sim::Scenario scenario{"issue-1855-one-optional", sim::DciProfile::StrictJit,
                         sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185501};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Serena, sim::Card::LatiasEx,
                sim::Card::EarthenVessel, sim::Card::Channeler,
                sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Crispin,
                sim::Card::RegidragoV, sim::Card::Oricorio};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Active Regidrago VSTAR makes Latias ex's setup role dead under the existing
  // strict-JIT selector. The optional slot should then replace the locked Item:
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Full Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine),
         "Existing Serena route did not resolve");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::LatiasEx),
         "Existing first-discard selection changed");
  expect(contains(after.discard, sim::Card::EarthenVessel),
         "Locked Item did not fill the optional Serena slot");
}

void test_permanent_lock_fix_does_not_open_new_serena_route() {
  sim::Scenario scenario{"issue-1855-selector-boundary", sim::DciProfile::StrictJit,
                         sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185502};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  state.hand = {sim::Card::ChaoticSwell, sim::Card::Serena,
                sim::Card::QuickBall, sim::Card::Fire,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Grass,
                sim::Card::Grass, sim::Card::HisuianHeavyBall,
                sim::Card::RegidragoVstar, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, state);

  // Issue #1855 concerns optional discards after Serena is already selected.
  // It must preserve the prior route selector when no acceptable mandatory cost
  // exists, including the live Celestial Roar setup state represented here:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(!sim::EngineTestAccess::play_serena(engine),
         "Optional-discard fix opened a new Serena route");
  expect(sim::EngineTestAccess::state(engine).hand == state.hand,
         "Rejected Serena projection mutated the hand");
}

void test_scheduled_lock_does_not_promote_currently_live_items() {
  sim::Scenario scenario{"issue-1855-scheduled-lock", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185503};
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

  // TurnTwoItem is still unlocked on turn one, so Earthen Vessel remains live:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Scheduled lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena did not resolve in the scheduled-lock control");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::EarthenVessel),
         "A currently live Item was treated as permanently locked");
}

void test_zero_draw_payload_completion_preserves_dead_items() {
  sim::Scenario scenario{"issue-1855-zero-draw", sim::DciProfile::StrictJit,
                         sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{185504};
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

  // The required payload consumes Serena's first slot. With six cards left after
  // that payment, another discard adds no draw and must remain held:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1855
  expect(sim::EngineTestAccess::play_serena(engine, true),
         "Serena did not complete the zero-draw strict-JIT payload route");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MegaDragonite),
         "Required Dragon payload was not discarded");
  expect(contains(after.hand, sim::Card::EarthenVessel) &&
             contains(after.hand, sim::Card::MysteriousTreasure) &&
             contains(after.hand, sim::Card::QuickBall),
         "Serena spent a dead Item without gaining a draw");
}

}  // namespace

int main() {
  try {
    test_full_and_combined_lock_refresh();
    test_one_locked_item_uses_optional_slot_after_existing_first_cost();
    test_permanent_lock_fix_does_not_open_new_serena_route();
    test_scheduled_lock_does_not_promote_currently_live_items();
    test_zero_draw_payload_completion_preserves_dead_items();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
