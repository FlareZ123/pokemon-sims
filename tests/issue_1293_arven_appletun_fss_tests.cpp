#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool play_burnet(Engine& engine) { return engine.play_professor_burnet(); }
  static bool play_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static void advance_turn(Engine& engine) {
    ++engine.state_.turn;
    engine.state_.supporter_used = false;
    engine.state_.manual_energy_used = false;
    engine.state_.retreat_used = false;
    engine.state_.stadium_used = false;
    engine.state_.turn_ended = false;
    engine.state_.discarded_this_turn.clear();
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

sim::State payload_only_state(std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Arven};
  state.deck = std::move(deck);
  return state;
}

void run_fss_burnet_route(const sim::LockMode locks, const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-1293-fss-burnet", sim::DciProfile::StrictJit,
                               locks, false, 4};
  std::mt19937_64 rng{seed};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state({sim::Card::ForestSealStone,
                                  sim::Card::ProfessorBurnet,
                                  sim::Card::Appletun,
                                  sim::Card::Grass}));

  // Arven can search Forest Seal Stone. Star Alchemy can then search any card,
  // including Professor Burnet, and Burnet can discard the Stage 1 Dragon Appletun
  // on the next modeled turn for Apex Dragon:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1293
  expect(sim::EngineTestAccess::play_arven(engine),
         "Arven should recognize the Appletun Forest Seal Stone route.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::ForestSealStone),
         "Arven should search Forest Seal Stone.");
  expect(sim::EngineTestAccess::attach_fss(engine),
         "Forest Seal Stone should attach to Regidrago VSTAR.");
  expect(sim::EngineTestAccess::use_fss(engine),
         "Star Alchemy should bank Professor Burnet.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::ProfessorBurnet),
         "Star Alchemy should search Professor Burnet.");

  sim::EngineTestAccess::advance_turn(engine);
  expect(sim::EngineTestAccess::play_burnet(engine),
         "Professor Burnet should discard Appletun next turn.");
  expect(contains(sim::EngineTestAccess::state(engine).discard,
                  sim::Card::Appletun),
         "Appletun should enter discard through Professor Burnet.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The Burnet discard should satisfy strict-JIT payload readiness.");
}

void test_unlocked_fss_burnet_route() {
  run_fss_burnet_route(sim::LockMode::None, 129301);
}

void test_item_lock_does_not_block_tool_burnet_route() {
  // Pokémon Tools remain separate from Items, and Professor Burnet is a Supporter:
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  run_fss_burnet_route(sim::LockMode::FullItem, 129302);
}

void test_rule_box_lock_does_not_block_star_alchemy() {
  // Forest Seal Stone grants Star Alchemy from the Tool, so Path-style Rule Box
  // Ability lock does not remove it from the attached Pokémon V:
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  run_fss_burnet_route(sim::LockMode::FullRuleBoxAbility, 129303);
}

void test_direct_blender_route() {
  const sim::Scenario scenario{"issue-1293-direct-blender",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{129304};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state({sim::Card::BrilliantBlender,
                                  sim::Card::Appletun,
                                  sim::Card::Grass}));

  // Arven may take Brilliant Blender, whose deck-discard effect can select Appletun:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1293
  expect(sim::EngineTestAccess::play_arven(engine),
         "Arven should recognize the direct Appletun Blender route.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::BrilliantBlender),
         "Arven should search Brilliant Blender.");
  expect(sim::EngineTestAccess::play_blender(engine),
         "Brilliant Blender should discard Appletun.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Blender should complete strict-JIT payload readiness.");
}

void expect_arven_holds(const sim::Scenario& scenario,
                        std::vector<sim::Card> deck,
                        const std::uint64_t seed,
                        const char* message) {
  std::mt19937_64 rng{seed};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, payload_only_state(std::move(deck)));
  expect(!sim::EngineTestAccess::play_arven(engine), message);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 1U && result.hand.front() == sim::Card::Arven &&
             result.discard.empty(),
         "A rejected route must preserve Arven and all zones.");
}

void test_holds_without_future_turn() {
  const sim::Scenario scenario{"issue-1293-no-future-turn",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  // Burnet is a Supporter and cannot be played after Arven on the same turn:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect_arven_holds(scenario,
                     {sim::Card::ForestSealStone, sim::Card::ProfessorBurnet,
                      sim::Card::Appletun, sim::Card::Grass},
                     129305,
                     "Arven must hold when no modeled turn remains for Burnet.");
}

void test_holds_without_forest_seal_stone() {
  const sim::Scenario scenario{"issue-1293-no-fss",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  // Arven cannot create the reported continuation when Forest Seal Stone is absent:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  expect_arven_holds(scenario,
                     {sim::Card::ProfessorBurnet, sim::Card::Appletun,
                      sim::Card::Grass},
                     129306,
                     "Arven must hold without Forest Seal Stone.");
}

void test_holds_without_professor_burnet() {
  const sim::Scenario scenario{"issue-1293-no-burnet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  // Star Alchemy cannot bank the next-turn discard Supporter when Burnet is absent:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  expect_arven_holds(scenario,
                     {sim::Card::ForestSealStone, sim::Card::Appletun,
                      sim::Card::Grass},
                     129307,
                     "Arven must hold without Professor Burnet.");
}

void test_supporter_lock_blocks_arven() {
  const sim::Scenario scenario{"issue-1293-supporter-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullSupporter, false, 4};
  // A Supporter lock prohibits Arven and Professor Burnet:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect_arven_holds(scenario,
                     {sim::Card::ForestSealStone, sim::Card::ProfessorBurnet,
                      sim::Card::Appletun, sim::Card::Grass},
                     129308,
                     "Supporter lock must block the Arven route.");
}

}  // namespace

int main() {
  try {
    test_unlocked_fss_burnet_route();
    test_item_lock_does_not_block_tool_burnet_route();
    test_rule_box_lock_does_not_block_star_alchemy();
    test_direct_blender_route();
    test_holds_without_future_turn();
    test_holds_without_forest_seal_stone();
    test_holds_without_professor_burnet();
    test_supporter_lock_blocks_arven();
    std::cout << "Issue 1293 Arven Appletun Forest Seal Stone tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
