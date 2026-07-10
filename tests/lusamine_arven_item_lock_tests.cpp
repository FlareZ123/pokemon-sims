#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool play_lusamine(Engine& engine) { return engine.play_lusamine(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_lusamine_recovers_arven_for_item_lock_fss_route() {
  using namespace sim;
  const Scenario scenario{"lusamine-arven-item-lock", DciProfile::StrictJit, LockMode::FullItem, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(146);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::Lusamine};
  state.discard = {Card::Arven, Card::Guzma};
  state.deck = {Card::RegidragoVstar, Card::ForestSealStone};

  // Lusamine can recover discarded Supporter cards: https://api.pokemontcg.io/v2/cards/sm4-96
  // Arven remains playable during Item lock and may choose Forest Seal Stone as its Tool target: https://api.pokemontcg.io/v2/cards/sv1-166 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // Forest Seal Stone then grants Star Alchemy to the attached Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-156
  EngineTestAccess::run_turn(engine);

  assert(state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Arven) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Lusamine) == 1);

  state.turn = 3;
  state.supporter_used = false;
  state.manual_energy_used = false;
  state.retreat_used = false;
  state.turn_ended = false;
  state.discarded_this_turn.clear();
  EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(EngineTestAccess::outcome(engine).used_fss);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Arven) == 1);
}

void test_lusamine_holds_with_only_one_eligible_target() {
  using namespace sim;
  const Scenario scenario{"lusamine-one-target", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(149);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::Lusamine};
  state.discard = {Card::ProfessorBurnet, Card::Grass};

  // Lusamine requires exactly 2 Supporter and/or Stadium cards from the public
  // discard pile. One eligible target does not satisfy the printed effect:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  assert(!EngineTestAccess::play_lusamine(engine));
  assert(!state.supporter_used);
  assert(state.hand.size() == 1U && state.hand.front() == Card::Lusamine);
  assert(state.discard.size() == 2U && contains(state.discard, Card::ProfessorBurnet) &&
         contains(state.discard, Card::Grass));
}

void test_lusamine_accepts_two_copies_of_one_eligible_card() {
  using namespace sim;
  const Scenario scenario{"lusamine-duplicate-target", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(150);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::Lusamine};
  state.discard = {Card::ProfessorBurnet, Card::ProfessorBurnet};

  // The effect requires two cards, while allowing any combination of Supporter and
  // Stadium cards. Two copies of the same eligible Supporter are therefore legal:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  assert(EngineTestAccess::play_lusamine(engine));
  assert(state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::ProfessorBurnet) == 2);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Lusamine) == 1);
}

void test_roseanne_recovers_vstar_into_evolution_incense_route() {
  using namespace sim;
  const Scenario scenario{"roseanne-vstar-incense", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(147);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::RoseannesBackup, Card::EvolutionIncense};
  state.deck = {Card::Grass};
  state.prizes = {Card::RegidragoVstar, Card::RegidragoVstar};
  state.discard = {Card::RegidragoVstar, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine, true);

  // Roseanne's Backup may shuffle a Pokémon from discard into the deck. Evolution
  // Incense then searches that Evolution Pokémon, and the prior-turn Regidrago V may evolve:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);

  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoVstar);
  assert(state.supporter_used);
  assert(contains(state.discard, Card::RoseannesBackup));
  assert(contains(state.discard, Card::EvolutionIncense));
  assert(!contains(state.discard, Card::RegidragoVstar));
}

void test_roseanne_holds_without_a_payable_post_recovery_search() {
  using namespace sim;
  const Scenario scenario{"roseanne-unpayable-search", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(148);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::RoseannesBackup, Card::MysteriousTreasure};
  state.deck = {Card::Grass};
  state.prizes = {Card::RegidragoVstar, Card::RegidragoVstar};
  state.discard = {Card::RegidragoVstar, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine, true);

  // Mysterious Treasure needs another card from hand as its discard cost. Once
  // Roseanne's Backup leaves hand, no payable continuation remains:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sm6-113
  EngineTestAccess::run_turn(engine);

  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoV);
  assert(!state.supporter_used);
  assert(contains(state.hand, Card::RoseannesBackup));
  assert(contains(state.hand, Card::MysteriousTreasure));
  assert(contains(state.discard, Card::RegidragoVstar));
}

}  // namespace

int main() {
  test_lusamine_recovers_arven_for_item_lock_fss_route();
  test_lusamine_holds_with_only_one_eligible_target();
  test_lusamine_accepts_two_copies_of_one_eligible_card();
  test_roseanne_recovers_vstar_into_evolution_incense_route();
  test_roseanne_holds_without_a_payable_post_recovery_search();
  return 0;
}
