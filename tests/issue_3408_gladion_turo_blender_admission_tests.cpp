#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1595_known_grass_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1}};
  state.hand = {sim::Card::Gladion, sim::Card::BrilliantBlender,
                sim::Card::ProfessorTuro};
  state.deck = {sim::Card::Dragapult, sim::Card::RegidragoV,
                sim::Card::Fire};
  state.prizes = {sim::Card::Grass, sim::Card::FieldBlower,
                  sim::Card::Arven, sim::Card::Pineco,
                  sim::Card::Crispin, sim::Card::QuickBall};
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, const sim::LockMode lock,
                        const int max_turn, const int turn,
                        const bool k1 = true) {
  const sim::Scenario scenario{"issue-3408", dci, lock, false, max_turn};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{3408};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(turn), k1);
  return engine;
}

void test_relative_horizon_profiles_and_rulebox_lock() {
  // Gladion is the current Supporter; Turo is the following-turn Supporter; Blender
  // is the following-turn Item. Rule Box Ability lock does not suppress this package.
  // Gladion / Turo / Blender: https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sv4-171 https://api.pokemontcg.io/v2/cards/sv8-164
  // Advanced Trainer and Energy procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // JIT and lock policy / confirmed regression: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/issues/3408
  for (const int turn : {2, 3, 4}) {
    sim::Engine engine = make_engine(sim::DciProfile::MatchupFlexJit,
                                     sim::LockMode::None, turn + 1, turn);
    expect(sim::EngineTestAccess::route_available(engine),
           "A legal Gladion-Turo-Blender route inside horizon was rejected.");
  }

  sim::Engine strict = make_engine(sim::DciProfile::StrictJit,
                                   sim::LockMode::None, 3, 2);
  expect(sim::EngineTestAccess::route_available(strict),
         "StrictJit did not share same-ready-turn Blender timing.");

  sim::Engine rulebox = make_engine(sim::DciProfile::MatchupFlexJit,
                                    sim::LockMode::FullRuleBoxAbility, 3, 2);
  expect(sim::EngineTestAccess::route_available(rulebox),
         "Rule Box Ability lock incorrectly blocked a Trainer-only continuation.");
}

void test_projected_trainer_locks_and_horizon() {
  // TurnTwoItem starts on T2 and remains active, so a legal T1-going-second Gladion
  // state cannot project a T2 Brilliant Blender play through that scheduled lock.
  // Persistent Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Supporter and Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L404
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/3408
  sim::Engine projected_item_lock =
      make_engine(sim::DciProfile::MatchupFlexJit,
                  sim::LockMode::TurnTwoItem, 2, 1);
  expect(!sim::EngineTestAccess::route_available(projected_item_lock),
         "The route projected Brilliant Blender through the scheduled T2 Item lock.");

  sim::Engine supporter_lock =
      make_engine(sim::DciProfile::MatchupFlexJit,
                  sim::LockMode::FullSupporter, 3, 2);
  expect(!sim::EngineTestAccess::route_available(supporter_lock),
         "Full Supporter lock admitted Gladion and projected Turo.");

  sim::Engine full_item = make_engine(sim::DciProfile::MatchupFlexJit,
                                      sim::LockMode::FullItem, 3, 2);
  expect(!sim::EngineTestAccess::route_available(full_item),
         "Full Item lock admitted the Blender continuation.");

  sim::Engine expired = make_engine(sim::DciProfile::MatchupFlexJit,
                                    sim::LockMode::None, 3, 3);
  expect(!sim::EngineTestAccess::route_available(expired),
         "The two-turn route was admitted without a following scenario turn.");
}

void test_knowledge_profile_and_resource_gates_remain() {
  // K1 and the held Turo resource remain required; NoDiscardControl keeps its
  // earlier-banking policy rather than entering this same-ready-turn continuation.
  // K1 / JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/3408
  sim::Engine k0 = make_engine(sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, 3, 2, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the known-prized-Grass route.");

  sim::Engine no_control = make_engine(sim::DciProfile::NoDiscardControl,
                                       sim::LockMode::None, 3, 2);
  expect(!sim::EngineTestAccess::route_available(no_control),
         "NoDiscardControl entered the strict same-ready-turn continuation.");

  sim::Engine missing_turo = make_engine(sim::DciProfile::MatchupFlexJit,
                                         sim::LockMode::None, 3, 2);
  sim::State missing = route_state(2);
  missing.hand.erase(std::remove(missing.hand.begin(), missing.hand.end(),
                                 sim::Card::ProfessorTuro),
                     missing.hand.end());
  sim::EngineTestAccess::set_state(missing_turo, std::move(missing), true);
  expect(!sim::EngineTestAccess::route_available(missing_turo),
         "The route was admitted without Professor Turo's Scenario.");
}
}  // namespace

int main() {
  test_relative_horizon_profiles_and_rulebox_lock();
  test_projected_trainer_locks_and_horizon();
  test_knowledge_profile_and_resource_gates_remain();
}
