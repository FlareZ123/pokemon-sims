#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool prized_latias_gladion_route(const Engine& engine) {
    return engine.k1_prized_latias_gladion_promotion_route();
  }
};

}  // namespace sim

namespace {

sim::State prized_latias_gladion_fixture(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 0, 2, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Fire,
      sim::Card::QuickBall,
      sim::Card::MysteriousTreasure,
      sim::Card::MysteriousTreasure,
  };
  state.deck = {
      sim::Card::Gladion,
      sim::Card::DialgaGX,
      sim::Card::RegidragoV,
      sim::Card::Grass,
  };
  state.prizes = {
      sim::Card::LatiasEx,
      sim::Card::HisuianHeavyBall,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::PathToPeak,
      sim::Card::EarthenVessel,
  };
  return state;
}

void configure_engine(sim::Engine& engine, const int turn) {
  sim::EngineTestAccess::state(engine) = prized_latias_gladion_fixture(turn);
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_turn_one_opening_regidrago_cannot_project_vstar_evolution() {
  // Going second permits a Supporter on turn 1, so this fixture isolates the
  // evolution boundary that #4112 exposed rather than failing on Supporter timing:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"issue-4112-t1", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{4112};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  configure_engine(engine, 1);

  // A-05 forbids evolution during either player's first turn even though the
  // opening Regidrago V has entered_turn == 0. Regidrago VSTAR evolves from it:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/4112
  assert(!sim::EngineTestAccess::prized_latias_gladion_route(engine));
}

void test_turn_two_opening_regidrago_keeps_valid_gladion_route() {
  const sim::Scenario scenario{"issue-4112-t2", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{4113};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  configure_engine(engine, 2);

  // On turn 2 the same setup-era Regidrago V satisfies both legal evolution
  // boundaries. The #2293 K1 Gladion -> prized Latias ex route remains admitted:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/2293
  assert(sim::EngineTestAccess::prized_latias_gladion_route(engine));
}

}  // namespace

int main() {
  test_turn_one_opening_regidrago_cannot_project_vstar_evolution();
  test_turn_two_opening_regidrago_keeps_valid_gladion_route();
  return 0;
}
