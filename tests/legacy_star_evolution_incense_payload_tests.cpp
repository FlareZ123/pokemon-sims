#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_earthen_vessel(Engine& engine) { return engine.play_earthen_vessel(false); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_legacy_star_recovers_evolution_incense_payload_bridge() {
  using namespace sim;
  const Scenario scenario{"legacy-star-evolution-incense-payload", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(168);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.deck = {Card::MegaDragonite, Card::Grass, Card::Fire, Card::Crispin,
                Card::Arven, Card::Dipplin, Card::EvolutionIncense, Card::MysteriousTreasure};

  // Legacy Star can return up to two discarded cards. Evolution Incense can then
  // fetch an Evolution Dragon payload, and Mysterious Treasure can discard that
  // fetched Dragon from hand as its cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  EngineTestAccess::run_turn(engine);

  assert(contains(state.discard, Card::EvolutionIncense));
  assert(contains(state.discard, Card::MysteriousTreasure));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

void test_legacy_star_ignores_item_lock_dead_hand_item() {
  using namespace sim;
  const Scenario scenario{"legacy-star-item-lock-dead-hand-item", DciProfile::StrictJit, LockMode::FullItem, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(177);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::MysteriousTreasure};
  state.deck = {Card::Grass, Card::Fire, Card::Crispin, Card::Arven,
                Card::Dipplin, Card::MawileGX, Card::MegaDragonite};

  // Budew's Item-lock shape prevents playing Item cards from hand, so the held
  // Mysterious Treasure is not a live payload outlet. Legacy Star can still use
  // Regidrago VSTAR's Ability to discard the top 7 cards, and a same-turn Mega
  // Dragonite ex discard satisfies the strict-JIT Apex Dragon payload axis:
  // https://api.pokemontcg.io/v2/cards/me2pt5-16
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  EngineTestAccess::run_turn(engine);

  assert(contains(state.hand, Card::MysteriousTreasure));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

void test_legacy_star_ignores_spent_supporter_burnet() {
  using namespace sim;
  const Scenario scenario{"legacy-star-spent-supporter-burnet", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(178);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::ProfessorBurnet};
  state.supporter_used = true;
  state.deck = {Card::Grass, Card::Fire, Card::Crispin, Card::Arven,
                Card::Dipplin, Card::MawileGX, Card::MegaDragonite};

  // Professor Burnet is a Supporter, and the player may play only one Supporter
  // during the turn. After that slot is spent, Burnet is not a live payload outlet;
  // Legacy Star can still discard Mega Dragonite ex from the top 7 cards:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  EngineTestAccess::run_turn(engine);

  assert(contains(state.hand, Card::ProfessorBurnet));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

void test_legacy_star_recovers_vessel_bridge_after_supporter_use() {
  using namespace sim;
  const Scenario scenario{"legacy-star-vessel-after-supporter", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(207);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.supporter_used = true;
  state.deck = {Card::Fire, Card::MawileGX, Card::Channeler, Card::Arven,
                Card::Crispin, Card::Dipplin, Card::EarthenVessel, Card::MegaDragonite};

  // Legacy Star may recover any two cards discarded by its top-seven effect:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // With the Supporter slot spent, Crispin and Arven cannot be played. Earthen
  // Vessel plus Dipplin is live because Vessel discards another card and searches
  // the remaining Fire Energy for the unused manual attachment:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(contains(state.hand, Card::EarthenVessel));
  assert(contains(state.hand, Card::Dipplin));
  assert(!contains(state.hand, Card::Crispin));
  assert(!contains(state.hand, Card::Arven));
  assert(EngineTestAccess::play_earthen_vessel(engine));
  assert(contains(state.hand, Card::Fire));
  assert(EngineTestAccess::attach_manual(engine));
  EngineTestAccess::record_ready(engine);

  assert(state.active && state.active->grass == 2 && state.active->fire == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

void test_legacy_star_still_recovers_crispin_with_supporter_available() {
  using namespace sim;
  const Scenario scenario{"legacy-star-live-crispin", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(208);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.deck = {Card::Fire, Card::MawileGX, Card::Channeler, Card::Guzma,
                Card::Crispin, Card::ProfessorTuro, Card::TeamYellsCheer, Card::MegaDragonite};

  // Crispin remains a legal Legacy Star recovery while the Supporter play is still
  // available: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(contains(state.hand, Card::Crispin));
}

void test_team_yell_recovers_vstar_into_evolution_incense_route() {
  using namespace sim;
  const Scenario scenario{"team-yell-vstar-incense", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(209);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::TeamYellsCheer, Card::EvolutionIncense};
  state.deck = {Card::Grass};
  state.prizes = {Card::RegidragoVstar, Card::RegidragoVstar};
  state.discard = {Card::RegidragoVstar, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine, true);

  // Team Yell's Cheer restores the known discarded VSTAR, Evolution Incense finds
  // that Evolution Pokémon, and the Regidrago V from the prior turn may evolve:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);

  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoVstar);
  assert(state.supporter_used);
  assert(contains(state.discard, Card::TeamYellsCheer));
  assert(contains(state.discard, Card::EvolutionIncense));
  assert(!contains(state.discard, Card::RegidragoVstar));
}

void test_team_yell_holds_without_a_payable_post_recovery_search() {
  using namespace sim;
  const Scenario scenario{"team-yell-unpayable-search", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(210);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::TeamYellsCheer, Card::MysteriousTreasure};
  state.deck = {Card::Grass};
  state.prizes = {Card::RegidragoVstar, Card::RegidragoVstar};
  state.discard = {Card::RegidragoVstar, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine, true);

  // Mysterious Treasure requires another hand card as its discard cost. After Team
  // Yell's Cheer is played, no such card remains, so the recovery route must be held:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/sm6-113
  EngineTestAccess::run_turn(engine);

  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoV);
  assert(!state.supporter_used);
  assert(contains(state.hand, Card::TeamYellsCheer));
  assert(contains(state.hand, Card::MysteriousTreasure));
  assert(contains(state.discard, Card::RegidragoVstar));
}

}  // namespace

int main() {
  test_legacy_star_recovers_evolution_incense_payload_bridge();
  test_legacy_star_ignores_item_lock_dead_hand_item();
  test_legacy_star_ignores_spent_supporter_burnet();
  test_legacy_star_recovers_vessel_bridge_after_supporter_use();
  test_legacy_star_still_recovers_crispin_with_supporter_available();
  test_team_yell_recovers_vstar_into_evolution_incense_route();
  test_team_yell_holds_without_a_payable_post_recovery_search();
  return 0;
}
