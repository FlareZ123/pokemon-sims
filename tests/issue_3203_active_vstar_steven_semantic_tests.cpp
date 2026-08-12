#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static bool route(const Engine& engine) {
    return engine.late_steven_active_vstar_crispin_treasure_route_available();
  }
};
}  // namespace sim

namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn - 1, 0, 1};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Crispin, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::BrilliantBlender,
                sim::Card::EarthenVessel, sim::Card::Dragapult};
  return state;
}

bool available(sim::DciProfile dci, sim::LockMode locks, bool going_first,
               int turn, int max_turn, bool known = true) {
  std::mt19937_64 rng{3203};
  sim::Scenario scenario{"issue-3203", dci, locks, going_first, max_turn};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, route_state(turn), known);
  return sim::EngineTestAccess::route(engine);
}

void test_semantic_admission() {
  // Steven, Crispin, Treasure, and the manual attachment have the same physical
  // legality in these equivalent public states.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 3),
         "historical witness rejected");
  expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None, true, 2, 3),
         "MatchupFlexJit rejected");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None, false, 2, 3),
         "going-second rejected");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None, false, 3, 4),
         "later equivalent turn rejected");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility,
                   true, 2, 3),
         "Rule Box Ability lock rejected Trainer route");
}

void test_real_blockers() {
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem,
                    true, 2, 3), "current Item lock admitted route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                    true, 2, 3), "Supporter lock admitted route");
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                    true, 2, 3), "non-JIT profile admitted route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, 2, 2), "expired horizon admitted route");
  // K0 cannot inspect hidden deck identities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, 2, 3, false), "K0 admitted K1 route");
}
}  // namespace

int main() {
  try {
    test_semantic_admission();
    test_real_blockers();
    std::cout << "Issue 3203 active-VSTAR semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
