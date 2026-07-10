#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool play_professor_burnet(Engine& engine) { return engine.play_professor_burnet(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_policy_unavailable_quick_ball() {
  // Quick Ball requires discarding another card before its Basic-Pokémon search:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Strict-JIT policy preserves the pending Gladion Prize-recovery connector:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
  // Gladion may exchange itself with a card from the Prize cards:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-policy-unavailable-quick-ball", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(49);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::QuickBall};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be played when Quick Ball has no policy-admissible discard cost");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoV) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must exchange with the prized Regidrago V");
  }
}

void test_blind_gladion_ignores_unpayable_ultra_ball() {
  const sim::Scenario scenario{"gladion-unpayable-ultra-basic", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{227};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::UltraBall};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball requires two other cards, so the held copy is not a live Basic
  // search route. Gladion may exchange itself for the prized Regidrago V:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm4-95
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion should recover Regidrago V when Ultra Ball has no two-card cost.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoV) ||
      !contains(after.hand, sim::Card::UltraBall) ||
      !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion should exchange for Regidrago V and preserve the unpayable Ultra Ball.");
  }
}

void test_blind_gladion_holds_for_payable_ultra_ball() {
  const sim::Scenario scenario{"gladion-payable-ultra-basic", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{228};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::UltraBall,
                sim::Card::MawileGX, sim::Card::Guzma};
  state.deck = {sim::Card::RegidragoV};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Matchup-flex policy permits the two visible matchup cards as legal costs, so
  // Ultra Ball can directly search Regidrago V and Gladion should be preserved:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm4-95
  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion should be held while a payable Ultra Ball can find Regidrago V.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Gladion) ||
      !contains(after.hand, sim::Card::UltraBall)) {
    throw std::runtime_error("The live Ultra Ball route should leave both connectors in hand before Item play.");
  }
}

void test_item_locked_forest_seal_stone_is_still_a_vstar_route() {
  // Pokémon Tool cards received an erratum that makes them a category separate from Item cards:
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // Forest Seal Stone must be attached to a Pokémon V before its VSTAR Power can be used:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  const sim::Scenario scenario{"gladion-item-locked-fss", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(58);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Gladion, sim::Card::ForestSealStone};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);

  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be held because Forest Seal Stone remains playable through Item lock");
  }
}

void test_item_locked_forest_seal_stone_finds_burnet_for_payload() {
  // Pokémon Tools are separate from Item cards, so Item lock does not prohibit attaching FSS:
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // Star Alchemy searches for any card, then Professor Burnet searches and discards deck cards:
  // https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  const sim::Scenario scenario{"item-locked-fss-burnet-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(60);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::ForestSealStone};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, state);

  if (!sim::EngineTestAccess::attach_fss(engine)) {
    throw std::runtime_error("Forest Seal Stone must attach through Item lock");
  }
  if (!sim::EngineTestAccess::use_fss(engine)) {
    throw std::runtime_error("Attached Forest Seal Stone must use Star Alchemy");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::ProfessorBurnet)) {
    throw std::runtime_error("Star Alchemy must find Professor Burnet under Item lock");
  }
  if (!sim::EngineTestAccess::play_professor_burnet(engine)) {
    throw std::runtime_error("Professor Burnet must resolve after the Forest Seal Stone search");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Professor Burnet must discard the payload found through Forest Seal Stone");
  }
}

void test_forest_seal_stone_without_an_open_tool_slot() {
  // A Pokémon can have only one Pokémon Tool attached, so an attached Powerglass blocks
  // Forest Seal Stone from being attached to that Regidrago V:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Gladion may exchange itself with a card from the Prize cards:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-fss-without-tool-slot", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(59);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::Powerglass};
  state.hand = {sim::Card::Gladion, sim::Card::ForestSealStone};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be played when Forest Seal Stone has no legal attachment target");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must exchange with the prized Regidrago VSTAR");
  }
}

void test_item_locked_evolution_incense_is_not_a_vstar_route() {
  // Evolution Incense is an Item, so Item lock makes it unavailable from hand:
  // https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/me2pt5-16
  // Gladion may exchange itself with a card from the Prize cards:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-item-locked-evolution-incense", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(61);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::EvolutionIncense};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be played when Item lock makes Evolution Incense unavailable");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must exchange with the prized Regidrago VSTAR under Item lock");
  }
}

void test_known_prized_vstar_makes_evolution_incense_dead() {
  // Evolution Incense can search only the deck, so it cannot recover a VSTAR that
  // a legal K1 inspection has proven is in the Prize cards:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // Gladion may exchange itself with that known Prize card:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-known-prized-vstar-evolution-incense", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(62);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Grass};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);
  sim::EngineTestAccess::set_deck_seen(engine);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be played when K1 proves Evolution Incense cannot find the VSTAR");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must exchange with the known prized Regidrago VSTAR");
  }
}

void test_known_prized_vstar_prefers_immediate_gladion_over_arven_fss() {
  // Gladion exchanges itself for a selected Prize card:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // Arven consumes the turn's Supporter play before Forest Seal Stone can search;
  // Star Alchemy searches the deck and cannot retrieve this known Prize card:
  // https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/swsh12-156
  // A player may play only one Supporter card during their turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  const sim::Scenario scenario{"gladion-known-prized-vstar-arven-fss", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{163};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Arven};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Grass};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Fire, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must immediately recover a known prized VSTAR instead of taking the delayed Arven route");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.supporter_used || !contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.hand, sim::Card::Arven) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must recover the VSTAR now and preserve the unused Arven connector");
  }
}

}  // namespace

int main() {
  test_policy_unavailable_quick_ball();
  test_blind_gladion_ignores_unpayable_ultra_ball();
  test_blind_gladion_holds_for_payable_ultra_ball();
  test_item_locked_forest_seal_stone_is_still_a_vstar_route();
  test_item_locked_forest_seal_stone_finds_burnet_for_payload();
  test_forest_seal_stone_without_an_open_tool_slot();
  test_item_locked_evolution_incense_is_not_a_vstar_route();
  test_known_prized_vstar_makes_evolution_incense_dead();
  test_known_prized_vstar_prefers_immediate_gladion_over_arven_fss();
  return 0;
}
