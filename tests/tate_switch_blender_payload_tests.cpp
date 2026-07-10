#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool play_tate_draw(Engine& engine) { return engine.play_tate_draw(); }
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

  // Tate & Liza can switch the Active with a Benched Pokémon, and Brilliant Blender
  // can then search and discard the current-turn Dragon payload as a later Item play:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(contains(state.discard, Card::TateLiza));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
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

}  // namespace

int main() {
  test_tate_switch_preserves_blender_payload_line();
  test_tate_draw_holds_when_no_card_can_enter_the_deck();
  test_tate_draw_uses_a_remaining_hand_card_with_empty_deck();
  test_professor_turo_returns_basic_active_and_promotes_complete_vstar();
  return 0;
}
