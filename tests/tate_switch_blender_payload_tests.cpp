#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void run_tactical_turn(Engine& engine) { engine.run_turn(); }
  static void run_full_turn(Engine& engine) {
    // Full-turn fixtures mirror Engine::run(): draw before tactical policy, then stop
    // when the mandatory draw ends the game:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
    const int turn = engine.state_.turn;
    engine.begin_turn(turn);
    if (!engine.state_.turn_ended) engine.run_turn();
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static bool play_tate_draw(Engine& engine) { return engine.play_tate_draw(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static bool play_turo_active_promotion_route(Engine& engine) {
    return engine.play_turo_active_promotion_route();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_tate_switch_preserves_blender_payload_line() {
  using namespace sim;
  const Scenario scenario{"tate-switch-blender-payload", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(164);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::TateLiza, Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite};

  // This is an explicit post-draw tactical state. Tate & Liza can switch the Active
  // with a Benched Pokémon, then Brilliant Blender can discard the Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  EngineTestAccess::run_tactical_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(contains(state.discard, Card::TateLiza));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

void test_incomplete_benched_vstar_preserves_tate_blender_line() {
  using namespace sim;
  const Scenario scenario{"tate-blender-incomplete-vstar", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(352);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 1, 0, Tool::None}};
  state.hand = {Card::Fire, Card::TateLiza, Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite};

  // This is an explicit post-draw tactical state. The manual attachment is once per
  // turn. Put Fire on the intended promoted attacker, then retain Tate and Blender
  // until a later Grass attachment completes Apex Dragon's GGF requirement:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  EngineTestAccess::run_tactical_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoV);
  assert(state.active->grass == 2 && state.active->fire == 0);
  assert(state.bench.size() == 1U);
  assert(state.bench.front().card == Card::RegidragoVstar);
  assert(state.bench.front().grass == 1 && state.bench.front().fire == 1);
  assert(state.manual_energy_used);
  assert(!state.supporter_used);
  assert(contains(state.hand, Card::TateLiza));
  assert(contains(state.hand, Card::BrilliantBlender));
  assert(state.deck.size() == 1U && state.deck.front() == Card::MegaDragonite);
  assert(state.discard.empty());
  assert(!EngineTestAccess::payload_ready(engine));
}

void test_ready_active_basic_does_not_hide_incomplete_tate_target() {
  using namespace sim;
  const Scenario scenario{"tate-blender-target-aware-energy", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(398);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None}};
  state.hand = {Card::TateLiza, Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite};

  // This is an explicit post-draw tactical state. The Active Basic's GGF does not
  // complete the Energy axis of the Benched VSTAR that Tate & Liza would promote:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  EngineTestAccess::run_tactical_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoV);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(state.bench.size() == 1U);
  assert(state.bench.front().card == Card::RegidragoVstar);
  assert(state.bench.front().grass == 1 && state.bench.front().fire == 1);
  assert(!state.supporter_used);
  assert(contains(state.hand, Card::TateLiza));
  assert(contains(state.hand, Card::BrilliantBlender));
  assert(state.deck.size() == 1U && state.deck.front() == Card::MegaDragonite);
  assert(state.discard.empty());
  assert(!EngineTestAccess::payload_ready(engine));
}

void test_blender_discards_every_payload_before_dipplin() {
  using namespace sim;
  const Scenario scenario{"blender-all-payloads", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(353);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                Card::DialgaGX, Card::Dipplin, Card::Arven};

  // Blender can discard five selected deck cards. Every modeled A/S Dragon adds an
  // Apex Dragon attack, so all four payloads take priority before Dipplin:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/sm5-100
  assert(EngineTestAccess::play_brilliant_blender(engine));
  assert(state.deck.size() == 1U && state.deck.front() == Card::Arven);
  for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                          Card::GoodraVstar, Card::DialgaGX,
                          Card::Dipplin}) {
    assert(contains(state.discard, card));
  }
}

void test_blender_uses_safe_remaining_selections_before_tate_refresh() {
  using namespace sim;
  const Scenario scenario{"blender-tate-deck-thinning", DciProfile::NoDiscardControl,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(338);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::BrilliantBlender, Card::TateLiza};
  state.deck = {Card::MegaDragonite, Card::Dipplin, Card::PathToPeak,
                Card::Guzma, Card::Channeler, Card::Fire,
                Card::RegidragoVstar, Card::Grass, Card::Arven,
                Card::LatiasEx};

  // Blender may choose up to five deck cards. In no-discard-control, the three
  // no-lock setup-dead cards are safe DCI selections before Tate & Liza shuffles
  // the hand and draws five:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  assert(EngineTestAccess::play_brilliant_blender(engine));
  assert(state.deck.size() == 5U);
  for (const Card card : {Card::MegaDragonite, Card::Dipplin,
                          Card::PathToPeak, Card::Guzma,
                          Card::Channeler}) {
    assert(contains(state.discard, card));
  }

  assert(EngineTestAccess::play_tate_draw(engine));
  assert(state.deck.empty());
  assert(contains(state.hand, Card::Fire));
}

void test_tate_draw_holds_when_no_card_can_enter_the_deck() {
  using namespace sim;
  const Scenario scenario{"tate-empty-refresh", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(333);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 0, 0, Tool::None};
  state.hand = {Card::TateLiza};
  state.deck.clear();

  // The draw mode would shuffle no cards and draw no cards, so the Trainer is known
  // to have no effect and must remain unplayed:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  assert(!EngineTestAccess::play_tate_draw(engine));
  assert(!state.supporter_used);
  assert(state.hand.size() == 1U && state.hand.front() == Card::TateLiza);
  assert(state.deck.empty());
  assert(state.discard.empty());
}

void test_tate_draw_uses_a_remaining_hand_card_with_empty_deck() {
  using namespace sim;
  const Scenario scenario{"tate-empty-deck-live-refresh", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(334);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 0, 0, Tool::None};
  state.hand = {Card::TateLiza, Card::Dipplin};
  state.deck.clear();

  // Tate & Liza remains legal because the other hand card is shuffled into the deck
  // before the draw instruction resolves:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  assert(EngineTestAccess::play_tate_draw(engine));
  assert(state.supporter_used);
  assert(state.hand.size() == 1U && state.hand.front() == Card::Dipplin);
  assert(state.deck.empty());
  assert(std::count(state.discard.begin(), state.discard.end(), Card::TateLiza) == 1);
}

void test_tate_draw_refreshes_an_empty_deck_when_search_item_cannot_follow() {
  using namespace sim;
  const Scenario scenario{"tate-empty-deck-held-payload", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(511);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::Oricorio, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::TateLiza, Card::MysteriousTreasure, Card::MegaDragonite,
                Card::Dipplin, Card::FieldBlower, Card::ChaoticSwell,
                Card::PathToPeak};
  state.deck.clear();

  // Mysterious Treasure cannot begin its required deck search with zero cards, so it
  // is not a live reason to suppress Tate & Liza's draw mode. Tate shuffles the six
  // remaining hand cards, draws five, and leaves one card for the next turn's draw:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
  assert(EngineTestAccess::play_tate_draw(engine));

  assert(state.supporter_used);
  assert(state.hand.size() == 5U);
  assert(state.deck.size() == 1U);
  assert(contains(state.discard, Card::TateLiza));
  assert(!contains(state.discarded_this_turn, Card::MegaDragonite));
}

void test_tate_preserves_one_card_deck_for_live_payload_search() {
  using namespace sim;
  const Scenario scenario{"tate-one-card-held-payload", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(512);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::Oricorio, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::TateLiza, Card::MysteriousTreasure, Card::MegaDragonite};
  state.deck = {Card::Grass};

  // A publicly nonempty deck keeps the held Mysterious Treasure route live. Tate may
  // preserve switch mode, then Treasure may discard the Dragon and begin its search:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(!EngineTestAccess::play_tate_draw(engine));
  assert(EngineTestAccess::play_tate_switch(engine));
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(EngineTestAccess::play_mysterious_treasure(engine));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

void test_professor_turo_returns_basic_active_and_promotes_complete_vstar() {
  using namespace sim;
  const Scenario scenario{"turo-active-promotion", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(307);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 0, 1, 0, Tool::Powerglass};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::ProfessorTuro};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Professor Turo's Scenario returns the selected Pokémon to hand and discards
  // its attached cards. Removing the Active Pokémon requires a Benched replacement,
  // so the complete Regidrago VSTAR can become Active without using the retreat:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_turo_active_promotion_route(engine));

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(state.bench.empty());
  assert(contains(state.hand, Card::TapuLeleGX));
  assert(contains(state.discard, Card::ProfessorTuro));
  assert(contains(state.discard, Card::Grass));
  assert(contains(state.discard, Card::Powerglass));
  assert(state.supporter_used);
  assert(!state.retreat_used);
}

void test_full_turn_draw_precedes_tactical_evolution() {
  using namespace sim;
  const Scenario scenario{"full-turn-draw-before-policy", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(489);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.deck = {Card::RegidragoVstar};

  // The player draws before taking tactical actions. Drawing the VSTAR from
  // deck.back() makes the subsequent legal evolution possible in the same full turn:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  EngineTestAccess::run_full_turn(engine);

  assert(!state.turn_ended);
  assert(state.deck.empty());
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(!contains(state.hand, Card::RegidragoVstar));
}

void test_empty_deck_draw_prevents_tactical_evolution() {
  using namespace sim;
  const Scenario scenario{"empty-deck-stops-before-policy", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(490);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::RegidragoVstar};
  state.deck.clear();

  // A player who cannot draw at the start of the turn loses before taking further
  // actions. The held VSTAR therefore remains in hand and the Active does not evolve:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
  EngineTestAccess::run_full_turn(engine);

  assert(state.turn_ended);
  assert(state.active && state.active->card == Card::RegidragoV);
  assert(contains(state.hand, Card::RegidragoVstar));
}

}  // namespace

int main() {
  test_tate_switch_preserves_blender_payload_line();
  test_incomplete_benched_vstar_preserves_tate_blender_line();
  test_ready_active_basic_does_not_hide_incomplete_tate_target();
  test_blender_discards_every_payload_before_dipplin();
  test_blender_uses_safe_remaining_selections_before_tate_refresh();
  test_tate_draw_holds_when_no_card_can_enter_the_deck();
  test_tate_draw_uses_a_remaining_hand_card_with_empty_deck();
  test_tate_draw_refreshes_an_empty_deck_when_search_item_cannot_follow();
  test_tate_preserves_one_card_deck_for_live_payload_search();
  test_professor_turo_returns_basic_active_and_promotes_complete_vstar();
  test_full_turn_draw_precedes_tactical_evolution();
  test_empty_deck_draw_prevents_tactical_evolution();
  return 0;
}
