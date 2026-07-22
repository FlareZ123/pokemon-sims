#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static void play_items(Engine& engine) {
    engine.play_items_until_stable(true);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State ready_payload_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  return state;
}

void test_treasure_route_preserves_arven_and_blender() {
  const sim::Scenario scenario{"issue-1343-treasure",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1343};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = ready_payload_state();
  state.hand = {sim::Card::Arven, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::DialgaGX, sim::Card::TapuLeleGX,
                sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Treasure can discard the public Dragon and legally search a Psychic or Dragon
  // target. Arven must not manufacture a redundant ACE SPEC route:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1343
  sim::EngineTestAccess::choose_supporter(engine);
  expect(!sim::EngineTestAccess::state(engine).supporter_used,
         "The held Treasure payload finish must preserve the Supporter play.");

  sim::EngineTestAccess::play_items(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::Arven),
         "Arven must remain in hand.");
  expect(contains(result.deck, sim::Card::BrilliantBlender),
         "Brilliant Blender must remain in deck.");
  expect(contains(result.discard, sim::Card::MysteriousTreasure) &&
             contains(result.discard, sim::Card::MegaDragonite),
         "Mysterious Treasure must use the held Dragon as its legal cost.");
  expect(!contains(result.discard, sim::Card::BrilliantBlender),
         "The ACE SPEC must not be consumed.");
}

void test_payable_ultra_ball_preserves_arven() {
  const sim::Scenario scenario{"issue-1343-ultra",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1344};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = ready_payload_state();
  state.hand = {sim::Card::Arven, sim::Card::UltraBall,
                sim::Card::MegaDragonite, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::TapuLeleGX, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball can pay the Dragon plus a redundant held VSTAR and search a
  // Pokémon. The Active VSTAR makes the extra copy policy-legal to discard:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://github.com/FlareZ123/pokemon-sims/issues/1343
  sim::EngineTestAccess::choose_supporter(engine);
  expect(!sim::EngineTestAccess::state(engine).supporter_used,
         "A payable Ultra Ball payload route must preserve Arven.");
}

void test_no_held_payload_keeps_arven_blender_route() {
  const sim::Scenario scenario{"issue-1343-no-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1345};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = ready_payload_state();
  state.hand = {sim::Card::Arven, sim::Card::MysteriousTreasure,
                sim::Card::Grass};
  state.deck = {sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::DialgaGX, sim::Card::TapuLeleGX,
                sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Without a held Dragon payload, the direct hand-discard finish does not exist.
  // Arven may still search Brilliant Blender as the live deck-to-discard route:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1343
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used,
         "Arven must remain live when no payload is held.");
  expect(contains(result.discard, sim::Card::Arven) &&
             contains(result.hand, sim::Card::BrilliantBlender),
         "Arven must search Brilliant Blender in the positive control.");
}

void test_target_dead_quick_ball_keeps_arven_blender_route() {
  const sim::Scenario scenario{"issue-1343-dead-quick-ball",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1346};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = ready_payload_state();
  state.hand = {sim::Card::Arven, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::GoodraVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball cannot be played when legal inspection proves no Basic target exists.
  // The target-dead Item must not suppress Arven's live Blender route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/1343
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used,
         "A target-dead Quick Ball must not suppress Arven.");
  expect(contains(result.hand, sim::Card::BrilliantBlender),
         "Arven must search the live Blender route.");
}

void test_unpayable_ultra_ball_keeps_arven_blender_route() {
  const sim::Scenario scenario{"issue-1343-unpayable-ultra",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1347};
  const auto recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = ready_payload_state();
  state.hand = {sim::Card::Arven, sim::Card::UltraBall,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::TapuLeleGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball needs two other cards. One held Dragon is insufficient, so Arven's
  // live Blender route must remain available:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1343
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used,
         "An unpayable Ultra Ball must not suppress Arven.");
  expect(contains(result.hand, sim::Card::BrilliantBlender),
         "Arven must search Blender when Ultra Ball lacks a second cost.");
}

}  // namespace

int main() {
  try {
    test_treasure_route_preserves_arven_and_blender();
    test_payable_ultra_ball_preserves_arven();
    test_no_held_payload_keeps_arven_blender_route();
    test_target_dead_quick_ball_keeps_arven_blender_route();
    test_unpayable_ultra_ball_keeps_arven_blender_route();
    std::cout << "Issue 1343 Arven held-payload probe passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
