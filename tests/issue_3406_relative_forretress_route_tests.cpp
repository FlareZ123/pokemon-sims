#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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
    return engine.issue_1744_quick_ball_forretress_route_available();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1}};
  state.hand = {sim::Card::SecretBox, sim::Card::ForretressEx,
                sim::Card::QuickBall, sim::Card::Dragapult,
                sim::Card::Arven, sim::Card::Crispin};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoV,
                sim::Card::Fire, sim::Card::Pineco};
  return state;
}

sim::Engine make_engine(const sim::LockMode lock, const int max_turn,
                        const int turn, const bool k1 = true) {
  const sim::Scenario scenario{"issue-3406", sim::DciProfile::MatchupFlexJit,
                               lock, true, max_turn};
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (deck == nullptr) throw std::runtime_error("Pineco recipe is unavailable.");
  static std::mt19937_64 rng{3406};
  sim::Engine engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(turn), k1);
  return engine;
}

void test_relative_turns_and_supporter_lock() {
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Lock policy / confirmed regression: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/issues/3406
  for (const int turn : {2, 3, 4}) {
    sim::Engine engine = make_engine(sim::LockMode::None, 4, turn);
    expect(sim::EngineTestAccess::route_available(engine),
           "A legal Quick Ball-Forretress route inside horizon was rejected.");
  }

  sim::Engine supporter_locked =
      make_engine(sim::LockMode::FullSupporter, 4, 2);
  expect(sim::EngineTestAccess::route_available(supporter_locked),
         "Supporter-only lock incorrectly blocked an Item-plus-Ability route.");

  sim::Engine expired = make_engine(sim::LockMode::None, 3, 4);
  expect(!sim::EngineTestAccess::route_available(expired),
         "The route was admitted after the scenario horizon expired.");
}

void test_actual_lock_and_knowledge_gates_remain() {
  // Quick Ball / Forretress ex: https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/3406
  sim::Engine item_locked = make_engine(sim::LockMode::FullItem, 4, 2);
  expect(!sim::EngineTestAccess::route_available(item_locked),
         "Item lock admitted Quick Ball.");

  sim::Engine ability_locked =
      make_engine(sim::LockMode::FullRuleBoxAbility, 4, 2);
  expect(!sim::EngineTestAccess::route_available(ability_locked),
         "Rule Box Ability lock admitted Exploding Energy.");

  sim::Engine k0 = make_engine(sim::LockMode::None, 4, 2, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the K1-only Forretress route.");
}
}  // namespace

int main() {
  test_relative_turns_and_supporter_lock();
  test_actual_lock_and_knowledge_gates_remain();
}
