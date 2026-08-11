#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool preserve_forest_action(const Engine& engine) {
    return engine.issue_3057_preserve_forest_stadium_action();
  }
  static int late_hand_count(const Engine& engine, const Card card) {
    return engine.hand_count_issue_3057(card);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) { return line.find(text) != std::string::npos; });
}
sim::Engine make_engine(const sim::LockMode locks, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  return sim::Engine({"issue-3064", sim::DciProfile::StrictJit, locks, false, 3},
                     sim::pineco_recipe(), rng, trace);
}
sim::State direct_state(const int turn = 2, const int pineco_turn = 2) {
  sim::State s;
  s.turn = turn;
  s.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0, sim::Tool::None};
  s.bench.push_back(sim::Pokemon{sim::Card::Pineco, pineco_turn, 0, 0, sim::Tool::None});
  s.hand = {sim::Card::FieldBlower, sim::Card::ChaoticSwell,
            sim::Card::ForestOfVitality, sim::Card::ForretressEx};
  s.deck = {sim::Card::Grass, sim::Card::Grass};
  return s;
}
sim::State deferred_state() {
  sim::State s = direct_state();
  s.active = sim::Pokemon{sim::Card::RegidragoVstar, 0, 0, 0, sim::Tool::None};
  s.hand.erase(std::remove(s.hand.begin(), s.hand.end(), sim::Card::ForestOfVitality), s.hand.end());
  s.discard = {sim::Card::ForestOfVitality};
  s.deck = {sim::Card::Grass, sim::Card::Grass,
            sim::Card::PathToPeak, sim::Card::WishfulBaton,
            sim::Card::BattleVipPass, sim::Card::UltraBall,
            sim::Card::QuickBall, sim::Card::EvolutionIncense,
            sim::Card::PokemonCommunication};
  return s;
}

void test_direct_forest_dominates_unlock_items() {
  std::mt19937_64 rng(306401);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, direct_state());
  // Forest may replace Path directly and enables this turn's Grass evolution, so
  // Field Blower first spends a connector without improving the route:
  // Forest / Path / Field Blower: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125
  // Core Stadium/evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
  // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::preserve_forest_action(engine), "direct Forest route not recognized");
  expect(sim::EngineTestAccess::late_hand_count(engine, sim::Card::FieldBlower) == 0,
         "Field Blower not dominated by held Forest");
  expect(sim::EngineTestAccess::late_hand_count(engine, sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell not dominated by held Forest");
  sim::EngineTestAccess::run_turn(engine);
  const auto& state = sim::EngineTestAccess::state(engine);
  expect(state.stadium == sim::Stadium::ForestOfVitality, "Forest was not played directly");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::FieldBlower) == 1,
         "Field Blower was consumed on dominated route");
  expect(trace_contains(trace, "Forest of Vitality allowed"), "Forest evolution trace missing");
}

void test_deferred_forest_uses_blower_then_legacy_star() {
  std::mt19937_64 rng(306402);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, deferred_state());
  // Field Blower removes Path without using the Stadium action; restored Legacy
  // Star may then recover the already-public Forest from discard:
  // Regidrago VSTAR / Field Blower / Path / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/me1-117
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Hidden-info policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::preserve_forest_action(engine), "deferred Forest route not recognized");
  expect(sim::EngineTestAccess::late_hand_count(engine, sim::Card::FieldBlower) == 1,
         "Field Blower hidden from deferred route");
  expect(sim::EngineTestAccess::late_hand_count(engine, sim::Card::ChaoticSwell) == 0,
         "Swell not deferred for public Forest recovery");
  sim::EngineTestAccess::run_turn(engine);
  const auto& state = sim::EngineTestAccess::state(engine);
  expect(state.vstar_power_used, "Deferred Forest recovery did not consume the VSTAR Power.");
  expect(state.stadium == sim::Stadium::ForestOfVitality, "Recovered Forest was not played");
  expect(trace_contains(trace, "recovered deferred Forest route"), "Legacy Star Forest trace missing");
}

void test_controls_keep_generic_unlocks() {
  std::mt19937_64 rng(306403);
  sim::Engine item_locked = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_state(item_locked, deferred_state());
  // Item lock makes Field Blower illegal: https://api.pokemontcg.io/v2/cards/sm2-125
  // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  expect(!sim::EngineTestAccess::preserve_forest_action(item_locked), "item lock admitted deferred route");
  expect(sim::EngineTestAccess::late_hand_count(item_locked, sim::Card::ChaoticSwell) == 1,
         "item lock hid the Stadium fallback");

  sim::Engine first_turn = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_state(first_turn, direct_state(1, 1));
  // Forest excludes the player's first turn: https://api.pokemontcg.io/v2/cards/me1-117
  expect(!sim::EngineTestAccess::preserve_forest_action(first_turn), "first-turn Forest route admitted");

  sim::Engine prior_pineco = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_state(prior_pineco, direct_state(2, 1));
  // Prior-turn Pineco already satisfies ordinary evolution timing: https://www.pokemon.com/us/pokemon-tcg/rules
  expect(!sim::EngineTestAccess::preserve_forest_action(prior_pineco), "prior-turn Pineco reserved Forest");
}
}  // namespace

int main() {
  try {
    test_direct_forest_dominates_unlock_items();
    test_deferred_forest_uses_blower_then_legacy_star();
    test_controls_keep_generic_unlocks();
    std::cout << "Issue 3064 Forest dominance tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
