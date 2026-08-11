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
  static const TrialOutcome& outcome(const Engine& engine) {
    return engine.outcome_;
  }
  static bool direct_forest_route(const Engine& engine) {
    return engine.issue_3064_direct_forest_route_available();
  }
  static bool deferred_forest_route(const Engine& engine) {
    return engine.issue_3064_deferred_forest_route_available();
  }
  static bool preserve_forest_action(const Engine& engine) {
    return engine.issue_3057_preserve_forest_stadium_action();
  }
  static int late_hand_count(const Engine& engine, const Card card) {
    return engine.hand_count_issue_3057(card);
  }
  static void run_turn(Engine& engine) { engine.run_turn_original(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State held_forest_state(const int turn = 2,
                             const int pineco_entered_turn = 2) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco,
                                     pineco_entered_turn, 0, 0,
                                     sim::Tool::None});
  state.hand = {sim::Card::FieldBlower, sim::Card::ChaoticSwell,
                sim::Card::ForestOfVitality, sim::Card::ForretressEx};
  state.deck = {sim::Card::Grass, sim::Card::Grass};
  return state;
}

sim::State deferred_forest_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::SecretBox, sim::Card::FieldBlower,
                sim::Card::ChaoticSwell, sim::Card::Grant,
                sim::Card::WishfulBaton, sim::Card::ErikasInvitation};
  state.deck = {
      sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
      sim::Card::Dawn, sim::Card::ForestOfVitality, sim::Card::Pineco,
      sim::Card::ForretressEx, sim::Card::Dragapult,
      sim::Card::RegidragoVstar, sim::Card::Fire,
      sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
      sim::Card::Grass, sim::Card::Grass,
  };
  return state;
}

sim::Engine make_engine(const sim::LockMode locks, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::Scenario scenario{"issue-3064", sim::DciProfile::StrictJit,
                               locks, false, 3};
  return sim::Engine(scenario, sim::pineco_recipe(), rng, trace);
}

void test_held_forest_replaces_path_directly() {
  std::mt19937_64 rng(306401);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, held_forest_state());

  // A held Forest is the shortest complete connector: Stadium replacement clears
  // Path and Forest simultaneously grants the current-turn Grass evolution rule.
  // Field Blower adds an unnecessary Item cost and must be preserved.
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Official Stadium / Item / evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::direct_forest_route(engine),
         "The held-Forest direct route was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell was not deferred for direct Forest replacement.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 0,
         "Field Blower was not preserved for the direct Forest route.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.path_lock_removed,
         "Direct Forest did not clear the modeled Path lock.");
  expect(state.stadium == sim::Stadium::ForestOfVitality,
         "Forest of Vitality did not replace Path directly.");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::FieldBlower) == 1,
         "The dominated Field Blower connector was spent.");
  expect(trace_contains(trace, "Played Forest of Vitality"),
         "The trace did not record direct Forest replacement.");
  expect(trace_contains(trace, "Forest of Vitality allowed"),
         "The current-turn Pineco did not use Forest-enabled evolution.");
}

void test_held_forest_remains_direct_through_item_lock() {
  std::mt19937_64 rng(306402);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_state(engine, held_forest_state());

  // Item lock blocks Field Blower but does not block Stadium cards. Forest can
  // still replace Path and unlock Exploding Energy on the same turn.
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Official Trainer procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::direct_forest_route(engine),
         "Item lock incorrectly disabled the direct Stadium route.");
  sim::EngineTestAccess::run_turn(engine);
  expect(sim::EngineTestAccess::state(engine).stadium ==
             sim::Stadium::ForestOfVitality,
         "Forest did not replace Path through Item lock.");
}

void test_k1_field_blower_secret_box_fetches_forest() {
  std::mt19937_64 rng(306403);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, deferred_forest_state(), true);

  // Forest is K1-known in deck. Field Blower may clear Path first because the
  // exact post-Blower hand still pays Secret Box's three-card cost and the full
  // Pineco ALS remains deterministic. Secret Box then searches Forest while the
  // Stadium action is still unused.
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Official Item / Stadium / search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 / DCI / connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Secret Box planner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1118_secret_box.inc
  // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::direct_forest_route(engine),
         "The deferred fixture unexpectedly had a direct Forest route.");
  expect(sim::EngineTestAccess::deferred_forest_route(engine),
         "The K1 Field Blower -> Secret Box -> Forest route was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell was not deferred for the payable Secret Box route.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 1,
         "Field Blower was hidden from the deferred route.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(std::count(state.discard.begin(), state.discard.end(),
                    sim::Card::FieldBlower) == 1,
         "Deferred route did not spend Field Blower on Path.");
  expect(sim::EngineTestAccess::outcome(engine).used_secret_box,
         "Deferred route did not use the proven Secret Box connector.");
  expect(state.stadium == sim::Stadium::ForestOfVitality,
         "Secret Box did not preserve the Stadium action for Forest.");
  expect(trace_contains(trace, "Field Blower discarded the modeled Path"),
         "Trace did not record deferred Field Blower removal.");
  expect(trace_contains(trace, "Secret Box discarded three other cards"),
         "Trace did not record the payable Secret Box route.");
}

void test_k0_hidden_forest_does_not_use_deferred_oracle() {
  std::mt19937_64 rng(306404);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_state(engine, deferred_forest_state(), false);

  // Before a legal inspection, the simulator must not use the actual hidden deck
  // identity to justify spending Field Blower for a future Forest search.
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::deferred_forest_route(engine),
         "K0 incorrectly used hidden Forest knowledge as a deferred oracle.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "K0 incorrectly suppressed the observable Chaotic Swell fallback.");
}

void test_item_lock_rejects_deferred_field_blower_route() {
  std::mt19937_64 rng(306405);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_state(engine, deferred_forest_state(), true);

  // Field Blower and Secret Box are Items, so the deferred route is illegal under
  // Item lock even though Forest itself would remain legal if it were already held.
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Official Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::deferred_forest_route(engine),
         "Item lock admitted the deferred Item chain.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "Item lock incorrectly hid the legal Stadium fallback.");
}

void test_timing_and_axis_negatives_keep_swell_available() {
  std::mt19937_64 rng(306406);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);

  sim::State first_turn = held_forest_state(1, 1);
  sim::EngineTestAccess::set_state(engine, std::move(first_turn), true);
  expect(!sim::EngineTestAccess::direct_forest_route(engine),
         "Forest entry-turn evolution was admitted on the first turn.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "First-turn timing incorrectly suppressed Chaotic Swell.");

  sim::State prior_turn = held_forest_state(2, 1);
  sim::EngineTestAccess::set_state(engine, std::move(prior_turn), true);
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "A prior-turn Pineco unnecessarily reserved Forest.");

  sim::State energy_complete = held_forest_state();
  energy_complete.active->grass = 2;
  energy_complete.active->fire = 1;
  sim::EngineTestAccess::set_state(engine, std::move(energy_complete), true);
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "A complete Energy axis unnecessarily reserved the Forest route.");
}

void test_chaotic_swell_only_removal_remains_fallback() {
  std::mt19937_64 rng(306407);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::State state = deferred_forest_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::FieldBlower),
                   state.hand.end());
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::SecretBox),
                   state.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // Without a legal direct Forest or deferred Field Blower route, Chaotic Swell
  // remains the modeled Path-removal fallback.
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "Fallback Chaotic Swell was incorrectly hidden.");
  sim::EngineTestAccess::run_turn(engine);
  expect(sim::EngineTestAccess::state(engine).stadium ==
             sim::Stadium::ChaoticSwell,
         "Chaotic Swell did not remain the fallback Path-removal channel.");
}

}  // namespace

int main() {
  try {
    test_held_forest_replaces_path_directly();
    test_held_forest_remains_direct_through_item_lock();
    test_k1_field_blower_secret_box_fetches_forest();
    test_k0_hidden_forest_does_not_use_deferred_oracle();
    test_item_lock_rejects_deferred_field_blower_route();
    test_timing_and_axis_negatives_keep_swell_available();
    test_chaotic_swell_only_removal_remains_fallback();
    std::cout << "Issue 3064 Forest route dominance tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}