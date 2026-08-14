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
    engine.prizes_revealed_ = false;
  }
  static std::vector<Card> refined_targets(const Engine& engine) {
    return engine.issue_3545_refined_battle_compressor_targets();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_bc_stages_grant_before_ready_turn_when_future_costs_are_public() {
  // Grant is safe to stage before the ready turn because the Dragon remains in hand.
  // The existing Grant recovery path waits until Active VSTAR + GGF are complete,
  // then discards that Dragon plus a currently proved ordinary DCI card on the actual
  // ready turn. Wishful Baton is stable dead DCI in this goldfish model.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
  // Wishful Baton: https://api.pokemontcg.io/v2/cards/sm3-128
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Existing Grant recovery owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
  std::mt19937_64 rng{3545144};
  sim::Scenario scenario{"issue-3545-grant", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 3};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1};
  state.hand = {sim::Card::BattleCompressor,
                sim::Card::MegaDragonite,
                sim::Card::RegidragoVstar,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::Grant,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::MysteriousTreasure,
                sim::Card::TapuLeleGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  const auto targets = sim::EngineTestAccess::refined_targets(engine);
  expect(std::find(targets.begin(), targets.end(), sim::Card::Grant) != targets.end(),
         "Battle Compressor failed to stage Grant for the proved future JIT outlet.");
  expect(std::find_if(targets.begin(), targets.end(), sim::is_payload) == targets.end(),
         "Strict-JIT Battle Compressor discarded the Dragon before the ready turn.");
}
}  // namespace

int main() {
  try {
    test_bc_stages_grant_before_ready_turn_when_future_costs_are_public();
    std::cout << "issue 3545 Grant-stage tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
