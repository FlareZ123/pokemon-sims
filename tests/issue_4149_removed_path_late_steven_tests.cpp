#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine, const bool value) {
    engine.deck_seen_ = value;
  }
  static bool late_steven_route(Engine& engine) {
    return engine.late_steven_has_known_t3_compression_route();
  }
};

}  // namespace sim

namespace {

sim::Engine make_engine(const sim::LockMode locks, std::mt19937_64& rng) {
  using namespace sim;
  const Scenario scenario{"issue-4149-removed-path", DciProfile::StrictJit,
                          locks, false, 5};
  return Engine(scenario, baseline_recipe(), rng);
}

void install_late_steven_fixture(sim::Engine& engine, const bool path_removed) {
  using namespace sim;
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.path_lock_removed = path_removed;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 1, 0, Tool::None}};
  state.hand = {Card::StevensResolve, Card::TateLiza, Card::QuickBall,
                Card::RegidragoVstar, Card::Fire, Card::Grass};
  state.deck = {Card::LatiasEx, Card::MysteriousTreasure,
                Card::MegaDragonite, Card::Grass, Card::Fire};
  EngineTestAccess::set_deck_seen(engine, true);
}

void test_removed_path_restores_late_steven_route() {
  using namespace sim;
  std::mt19937_64 rng(4149);
  Engine engine = make_engine(LockMode::FullRuleBoxAbility, rng);
  install_late_steven_fixture(engine, true);

  // Path suppresses Rule Box Abilities only while it remains in play. Once Field
  // Blower removes it, Latias ex's Skyliner is available again, while Quick Ball and
  // Steven's Resolve remain governed by their own Item and Supporter permissions.
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Stadium, Ability, Item, Supporter, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Canonical lock primitives: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4149
  assert(EngineTestAccess::late_steven_route(engine));
}

void test_live_path_still_blocks_latias_connector() {
  using namespace sim;
  std::mt19937_64 rng(4150);
  Engine engine = make_engine(LockMode::FullRuleBoxAbility, rng);
  install_late_steven_fixture(engine, false);

  // A live Path still suppresses Latias ex's Rule Box Ability, so the zero-Retreat
  // connector required by this banked Steven packet remains unavailable.
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Ability procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  assert(!EngineTestAccess::late_steven_route(engine));
}

void test_item_lock_still_blocks_quick_ball_step() {
  using namespace sim;
  std::mt19937_64 rng(4151);
  Engine engine = make_engine(LockMode::FullItem, rng);
  install_late_steven_fixture(engine, true);

  // Quick Ball is an Item, so an active Item lock makes this packet illegal even
  // though no Path-style Ability lock remains.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  assert(!EngineTestAccess::late_steven_route(engine));
}

void test_supporter_lock_still_blocks_steven() {
  using namespace sim;
  std::mt19937_64 rng(4152);
  Engine engine = make_engine(LockMode::FullSupporter, rng);
  install_late_steven_fixture(engine, true);

  // Steven's Resolve is a Supporter, so the route remains illegal under the
  // simulator's full Supporter-lock fixture.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Canonical lock primitive: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  assert(!EngineTestAccess::late_steven_route(engine));
}

}  // namespace

int main() {
  test_removed_path_restores_late_steven_route();
  test_live_path_still_blocks_latias_connector();
  test_item_lock_still_blocks_quick_ball_step();
  test_supporter_lock_still_blocks_steven();
  return 0;
}
