#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_fss_fetches_tate_for_held_one_discard_payload() {
  using namespace sim;
  const Scenario scenario{"fss-tate-held-payload", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(507);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::Oricorio, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::ForestSealStone}};
  state.hand = {Card::MysteriousTreasure, Card::MegaDragonite};
  state.deck = {Card::TateLiza, Card::Dipplin};
  EngineTestAccess::set_deck_seen(engine);

  // Star Alchemy may search Tate & Liza, Tate may promote the GGF-complete VSTAR,
  // and Mysterious Treasure may discard the held Dragon before searching Dipplin:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!EngineTestAccess::use_fss(engine) ||
      !contains(state.hand, Card::TateLiza) || state.vstar_power_used == false) {
    throw std::runtime_error("Star Alchemy should search Tate for the payable held-payload route.");
  }
  if (!EngineTestAccess::play_tate_switch(engine) || !state.active ||
      state.active->card != Card::RegidragoVstar) {
    throw std::runtime_error("Tate should promote the GGF-complete Regidrago VSTAR.");
  }
  if (!EngineTestAccess::play_mysterious_treasure(engine, true) ||
      !contains(state.discarded_this_turn, Card::MegaDragonite) ||
      !EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The held Item should establish the strict-JIT payload after Tate switches.");
  }
}

void test_fss_holds_held_payload_route_for_incomplete_target() {
  using namespace sim;
  const Scenario scenario{"fss-tate-held-payload-incomplete", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(508);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::Oricorio, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::ForestSealStone}};
  state.hand = {Card::MysteriousTreasure, Card::MegaDragonite};
  state.deck = {Card::TateLiza, Card::Dipplin};
  EngineTestAccess::set_deck_seen(engine);

  // The promoted VSTAR must already pay Apex Dragon's GGF cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (EngineTestAccess::use_fss(engine) || state.vstar_power_used ||
      !contains(state.deck, Card::TateLiza)) {
    throw std::runtime_error("Star Alchemy must be preserved for an Energy-incomplete Tate target.");
  }
}

void test_fss_holds_when_tate_would_empty_deck_before_payload_item() {
  using namespace sim;
  const Scenario scenario{"fss-tate-held-payload-empty-deck", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(509);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::Oricorio, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::ForestSealStone}};
  state.hand = {Card::MysteriousTreasure, Card::MegaDragonite};
  state.deck = {Card::TateLiza};
  EngineTestAccess::set_deck_seen(engine);

  // Searching the final Tate would leave Mysterious Treasure unable to begin its
  // required deck search after paying the payload discard cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  if (EngineTestAccess::use_fss(engine) || state.vstar_power_used ||
      !contains(state.deck, Card::TateLiza)) {
    throw std::runtime_error("Star Alchemy must be preserved when Tate would empty the deck before the held Item.");
  }
}

void test_fss_fetches_tate_when_blender_covers_payload() {
  using namespace sim;
  const Scenario scenario{"fss-tate-blender-payload", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(175);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::ForestSealStone};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::BrilliantBlender};
  state.deck = {Card::TateLiza, Card::MegaDragonite, Card::FieldBlower};

  // A turn begins with a mandatory draw. Field Blower is the harmless top card in
  // this no-lock fixture, so Tate & Liza and Mega Dragonite ex remain in the deck for
  // the route being tested: https://www.pokemon.com/us/pokemon-tcg/rules
  // Forest Seal Stone can search Tate & Liza for switch mode, then the already held
  // Brilliant Blender remains playable later that turn to discard the Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  EngineTestAccess::run_turn(engine);

  assert(contains(state.hand, Card::FieldBlower));
  assert(state.vstar_power_used);
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(contains(state.discard, Card::TateLiza));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

void test_fss_holds_when_tate_target_is_energy_incomplete() {
  using namespace sim;
  const Scenario scenario{"fss-tate-incomplete-promotion", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(400);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::ForestSealStone};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None}};
  state.hand = {Card::BrilliantBlender};
  state.deck = {Card::TateLiza, Card::MegaDragonite, Card::FieldBlower};

  // Preserve the intended two-card route after the mandatory draw by placing a
  // harmless no-lock Field Blower on top: https://www.pokemon.com/us/pokemon-tcg/rules
  // Star Alchemy is one per game. Tate & Liza would promote the Benched VSTAR, and
  // Brilliant Blender would spend the current-turn payload, while that attacker still
  // cannot pay Apex Dragon's GGF cost. Preserve the VSTAR Power and both route cards:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  EngineTestAccess::run_turn(engine);

  assert(contains(state.hand, Card::FieldBlower));
  assert(!state.vstar_power_used);
  assert(state.active && state.active->card == Card::RegidragoV);
  assert(state.bench.size() == 1U);
  assert(state.bench.front().card == Card::RegidragoVstar);
  assert(state.bench.front().grass == 1 && state.bench.front().fire == 1);
  assert(contains(state.hand, Card::BrilliantBlender));
  assert(contains(state.deck, Card::TateLiza));
  assert(contains(state.deck, Card::MegaDragonite));
  assert(state.discard.empty());
  assert(!EngineTestAccess::payload_ready(engine));
}

}  // namespace

int main() {
  test_fss_fetches_tate_for_held_one_discard_payload();
  test_fss_holds_held_payload_route_for_incomplete_target();
  test_fss_holds_when_tate_would_empty_deck_before_payload_item();
  std::cout << "FSS held-payload Tate tests passed" << std::endl;
  test_fss_fetches_tate_when_blender_covers_payload();
  test_fss_holds_when_tate_target_is_energy_incomplete();
  return 0;
}
