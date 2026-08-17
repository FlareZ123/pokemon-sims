#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool active_vstar_steven_route(const Engine& engine) {
    return engine.issue_3203_active_vstar_steven_crispin_treasure_route_available();
  }
};

}  // namespace sim

namespace {

sim::State active_vstar_fixture(const bool manual_energy_used) {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = manual_energy_used;
  // The modeled #3203 continuation starts from Regidrago VSTAR with Fire already
  // attached and zero Grass. Steven reserves Crispin + Grass; next turn Crispin
  // attaches one Grass and the refreshed manual attachment supplies the second.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Route specification: https://github.com/FlareZ123/pokemon-sims/issues/3203
  state.active =
      sim::Pokemon{sim::Card::RegidragoVstar, 0, 0, 1, sim::Tool::None};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::MysteriousTreasure,
      sim::Card::DialgaGX,
  };
  state.deck = {
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::QuickBall,
  };
  return state;
}

void configure_engine(sim::Engine& engine, const bool manual_energy_used) {
  sim::EngineTestAccess::state(engine) = active_vstar_fixture(manual_energy_used);
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_current_turn_manual_attachment_does_not_block_next_turn_route() {
  const sim::Scenario scenario{"issue-4130-current-attachment",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{4130};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  configure_engine(engine, true);

  // Steven's Resolve ends this turn, and the simulator resets the once-per-turn
  // manual attachment state before the following turn where the route needs it.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Advanced turn/attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Canonical turn reset: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Confirmed timing bug: https://github.com/FlareZ123/pokemon-sims/issues/4130
  assert(sim::EngineTestAccess::active_vstar_steven_route(engine));
}

void test_unspent_current_attachment_keeps_same_route() {
  const sim::Scenario scenario{"issue-4130-unspent-attachment",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{4131};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  configure_engine(engine, false);

  assert(sim::EngineTestAccess::active_vstar_steven_route(engine));
}

void test_next_turn_item_lock_still_blocks_treasure_finish() {
  const sim::Scenario scenario{"issue-4130-next-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 3};
  std::mt19937_64 rng{4132};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  configure_engine(engine, true);

  // The timing correction preserves the route's real following-turn dependency:
  // Mysterious Treasure must still be legal on the projected finish turn.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // T2 Item-lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Confirmed timing bug: https://github.com/FlareZ123/pokemon-sims/issues/4130
  assert(!sim::EngineTestAccess::active_vstar_steven_route(engine));
}

}  // namespace

int main() {
  test_current_turn_manual_attachment_does_not_block_next_turn_route();
  test_unspent_current_attachment_keeps_same_route();
  test_next_turn_item_lock_still_blocks_treasure_finish();
  return 0;
}
