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
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
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

void test_bc_thins_stable_dead_card_before_live_celestial_roar() {
  // Going second, an Energy-attached Regidrago V can use Celestial Roar. With no
  // deterministic route that makes the attack redundant, discarding a stable-dead
  // non-Energy card before the attack strictly increases Basic-Energy density while
  // preserving every live setup connector and Dragon.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  std::mt19937_64 rng{3545135};
  sim::Scenario scenario{"issue-3545-roar", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 3};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
  state.hand = {sim::Card::BattleCompressor};
  state.deck = {sim::Card::Dipplin,
                sim::Card::WishfulBaton,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure,
                sim::Card::QuickBall};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  const auto targets = sim::EngineTestAccess::refined_targets(engine);
  expect(std::find(targets.begin(), targets.end(), sim::Card::Dipplin) != targets.end(),
         "BC failed to thin stable-dead Dipplin before a live Celestial Roar.");
  expect(std::find_if(targets.begin(), targets.end(), sim::is_payload) == targets.end(),
         "Strict-JIT BC discarded a Dragon merely to improve Celestial Roar density.");
  expect(std::none_of(targets.begin(), targets.end(), [](const sim::Card card) {
           return card == sim::Card::MysteriousTreasure || card == sim::Card::QuickBall;
         }),
         "BC sacrificed a live connector for Celestial Roar density.");
}
}  // namespace

int main() {
  try {
    test_bc_thins_stable_dead_card_before_live_celestial_roar();
    std::cout << "issue 3545 Celestial Roar tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
