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
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
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

sim::State blender_state(const bool include_held_payload) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender, sim::Card::MysteriousTreasure,
                sim::Card::Grass};
  if (include_held_payload) state.hand.push_back(sim::Card::MegaDragonite);
  state.deck = {sim::Card::Dragapult, sim::Card::DialgaGX,
                sim::Card::TapuLeleGX, sim::Card::RegidragoV};
  return state;
}

void test_held_payload_outlet_preserves_blender() {
  const sim::Scenario scenario{"issue-1339-held-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1339};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, blender_state(true));

  // Mysterious Treasure can discard the public Mega Dragonite ex and complete the
  // payload axis. Direct Blender play would consume the singleton ACE SPEC and
  // discard additional deck payloads without improving the ready turn:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/976
  // https://github.com/FlareZ123/pokemon-sims/issues/1339
  expect(!sim::EngineTestAccess::play_brilliant_blender(engine),
         "Brilliant Blender must hold when a direct held-payload outlet is live.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::BrilliantBlender),
         "The singleton Brilliant Blender must remain in hand.");
  expect(contains(result.hand, sim::Card::MegaDragonite),
         "The public payload must remain available for Mysterious Treasure.");
  expect(contains(result.deck, sim::Card::Dragapult) &&
             contains(result.deck, sim::Card::DialgaGX),
         "Deck-resident payloads must remain available for later turns.");
}

void test_no_held_payload_preserves_blender_route() {
  const sim::Scenario scenario{"issue-1339-no-held-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1340};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, blender_state(false));

  // Without a public payload, Brilliant Blender remains the immediate deterministic
  // deck-to-discard route:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://github.com/FlareZ123/pokemon-sims/issues/1339
  expect(sim::EngineTestAccess::play_brilliant_blender(engine),
         "Brilliant Blender must remain live when no payload is held.");
  expect(contains(sim::EngineTestAccess::state(engine).discard,
                  sim::Card::BrilliantBlender),
         "The positive control must consume Brilliant Blender.");
}

void test_known_dead_quick_ball_does_not_suppress_blender() {
  const sim::Scenario scenario{"issue-1339-dead-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1341};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::Dragapult, sim::Card::GoodraVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A K1 Quick Ball with no Basic target cannot be played merely to discard its
  // payload cost. That dead outlet must not suppress the live Blender route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/1339
  expect(sim::EngineTestAccess::play_brilliant_blender(engine),
         "A known-dead Quick Ball must not preserve Brilliant Blender.");
}

void test_item_lock_blocks_both_item_routes() {
  const sim::Scenario scenario{"issue-1339-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::Item, false, 4};
  std::mt19937_64 rng{1342};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, blender_state(true));

  // Item lock prevents both Brilliant Blender and Mysterious Treasure from being
  // played. The selector must preserve the ACE SPEC without pretending the held
  // outlet is usable:
  // https://api.pokemontcg.io/v2/cards/me2pt5-16
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1339
  expect(!sim::EngineTestAccess::play_brilliant_blender(engine),
         "Item lock must block Brilliant Blender.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::BrilliantBlender),
         "Item lock must preserve Brilliant Blender in hand.");
}

}  // namespace

int main() {
  try {
    test_held_payload_outlet_preserves_blender();
    test_no_held_payload_preserves_blender_route();
    test_known_dead_quick_ball_does_not_suppress_blender();
    test_item_lock_blocks_both_item_routes();
    std::cout << "Issue 1339 Brilliant Blender held-payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
