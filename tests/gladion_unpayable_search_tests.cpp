#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
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

}  // namespace

int main() {
  test_policy_unavailable_quick_ball();
  test_item_locked_forest_seal_stone_is_still_a_vstar_route();
  test_item_locked_forest_seal_stone_finds_burnet_for_payload();
  test_forest_seal_stone_without_an_open_tool_slot();
  return 0;
}
