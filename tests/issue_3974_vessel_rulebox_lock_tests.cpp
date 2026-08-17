#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool run_search_items_one_step(Engine& engine, const bool permit_payload) {
    return engine.run_search_items_one_step(permit_payload);
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool banks_vessel_for_next_turn(const sim::LockMode lock) {
  using namespace sim;
  const Scenario scenario{"issue-3974-bank", DciProfile::StrictJit, lock, false, 3};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3974);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 0, 0, Tool::None, 0};
  state.hand = {Card::Crispin, Card::EarthenVessel, Card::DialgaGX};
  state.deck = {Card::Grass, Card::Grass, Card::Fire};
  EngineTestAccess::set_deck_seen(engine);

  EngineTestAccess::run_search_items_one_step(engine, true);
  return state.issue_1447_vessel_ready_turn == 3;
}

bool replays_banked_vessel(const sim::LockMode lock) {
  using namespace sim;
  const Scenario scenario{"issue-3974-replay", DciProfile::StrictJit, lock, false, 3};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3975);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 3;
  state.active = Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None, 0};
  state.hand = {Card::EarthenVessel, Card::DialgaGX};
  state.deck = {Card::Grass};
  state.issue_1447_vessel_ready_turn = 3;
  EngineTestAccess::set_deck_seen(engine);

  EngineTestAccess::run_search_items_one_step(engine, true);
  return !contains(state.hand, Card::EarthenVessel) &&
      contains(state.discard, Card::EarthenVessel) &&
      contains(state.discard, Card::DialgaGX);
}

void test_hold_uses_action_specific_lock_legality() {
  using namespace sim;
  // Crispin is a Supporter and Earthen Vessel is an Item. Rule Box Ability-only
  // suppression does not prohibit either action, while a projected Item lock
  // prohibits the held Vessel and FullSupporter prohibits the still-needed Crispin:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Supporter, Item, attachment, and attack procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Canonical lock primitives: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3974
  expect(banks_vessel_for_next_turn(LockMode::None),
         "Unlocked #1447 state did not bank Earthen Vessel.");
  expect(banks_vessel_for_next_turn(LockMode::FullRuleBoxAbility),
         "Rule Box Ability-only lock incorrectly blocked the #1447 Vessel hold.");
  expect(!banks_vessel_for_next_turn(LockMode::FullItem),
         "Full Item lock incorrectly banked a future Earthen Vessel play.");
  expect(!banks_vessel_for_next_turn(LockMode::TurnTwoItem),
         "Scheduled Item lock incorrectly banked a locked next-turn Vessel play.");
  expect(!banks_vessel_for_next_turn(LockMode::FullCombined),
         "Combined lock incorrectly banked a locked next-turn Vessel play.");
  expect(!banks_vessel_for_next_turn(LockMode::FullSupporter),
         "Supporter lock incorrectly admitted the pre-Crispin hold state.");
}

void test_ready_turn_replay_depends_on_item_legality_only() {
  using namespace sim;
  // The recorded continuation's ready-turn action is Earthen Vessel plus the
  // ordinary manual attachment and Apex Dragon. It does not replay Crispin or
  // require a Rule Box Pokémon Ability, so only Item legality from the modeled
  // lock family is relevant at this point:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item, attachment, and attack procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Canonical lock primitives: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3974
  expect(replays_banked_vessel(LockMode::None),
         "Unlocked #1447 continuation did not replay Earthen Vessel.");
  expect(replays_banked_vessel(LockMode::FullRuleBoxAbility),
         "Rule Box Ability-only lock incorrectly blocked the ready-turn Vessel.");
  expect(replays_banked_vessel(LockMode::FullSupporter),
         "A Supporter lock incorrectly blocked a ready-turn route that needs no Supporter.");
  expect(!replays_banked_vessel(LockMode::FullItem),
         "Full Item lock illegally replayed Earthen Vessel.");
  expect(!replays_banked_vessel(LockMode::TurnTwoItem),
         "Scheduled Item lock illegally replayed Earthen Vessel.");
  expect(!replays_banked_vessel(LockMode::FullCombined),
         "Combined lock illegally replayed Earthen Vessel.");
}

}  // namespace

int main() {
  try {
    test_hold_uses_action_specific_lock_legality();
    test_ready_turn_replay_depends_on_item_legality_only();
    std::cout << "Issue 3974 Vessel Rule Box lock tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
