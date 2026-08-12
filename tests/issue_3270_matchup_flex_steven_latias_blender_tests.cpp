#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool available(const Engine& engine) {
    return engine.issue_2622_steven_latias_blender_package_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Grass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::Fire};
  return state;
}

bool available(const sim::DciProfile dci, const sim::LockMode lock,
               const bool k1 = true) {
  const sim::Scenario selected{"issue-3270", dci, lock, true, 5};
  std::mt19937_64 rng(3270);
  sim::Engine engine(selected, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, route_state(), k1);
  return sim::EngineTestAccess::available(engine);
}

void test_profile_parity_and_boundaries() {
  // Steven's Resolve reserves VSTAR/Latias/Blender. The held Grass completes GGF
  // on the following turn, and Blender supplies the Dragon payload on that same
  // ready turn. Both same-ready-turn JIT profiles therefore share this route.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // JIT/K1/priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original route / profile bug: https://github.com/FlareZ123/pokemon-sims/issues/2622 https://github.com/FlareZ123/pokemon-sims/issues/3270
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None),
         "StrictJit route regressed");
  expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None),
         "MatchupFlexJit equivalent route was rejected");
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None),
         "NoDiscardControl entered the same-ready-turn package");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem),
         "Future Blender Item lock was ignored");
  expect(!available(sim::DciProfile::StrictJit,
                    sim::LockMode::FullRuleBoxAbility),
         "Required Latias Rule Box Ability lock was ignored");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None, false),
         "K0 entered the K1 package");
}
}  // namespace

int main() {
  test_profile_parity_and_boundaries();
  return 0;
}
