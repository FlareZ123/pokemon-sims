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
  static void set_state(Engine& engine, State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool direct_forest(const Engine& engine) {
    return engine.issue_3064_direct_forest_over_path_route();
  }
  static bool deferred_forest(const Engine& engine) {
    return engine.issue_3064_deferred_prized_forest_via_tapu_route();
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

sim::State direct_forest_state(const int turn = 2,
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

sim::State deferred_prized_forest_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 2, 0, 0,
                                     sim::Tool::None});
  state.hand = {sim::Card::FieldBlower, sim::Card::ChaoticSwell,
                sim::Card::TapuLeleGX, sim::Card::ForretressEx};
  state.deck = {sim::Card::Gladion, sim::Card::Grass, sim::Card::Grass};
  state.prizes = {sim::Card::ForestOfVitality, sim::Card::Fire,
                  sim::Card::RegidragoVstar, sim::Card::QuickBall,
                  sim::Card::MysteriousTreasure, sim::Card::Crispin};
  return state;
}

sim::Engine make_engine(const sim::LockMode locks, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::Scenario scenario{"issue-3064", sim::DciProfile::StrictJit,
                               locks, false, 3};
  return sim::Engine(scenario, sim::pineco_recipe(), rng, trace);
}

void test_held_forest_directly_replaces_path() {
  std::mt19937_64 rng(306401);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, direct_forest_state());

  // A held Forest is the complete one-card Stadium route: it replaces Path,
  // restores Rule Box Abilities, remains active, and enables current-turn Pineco
  // evolution. Spending Field Blower first is dominated and Swell spends the one
  // Stadium action on the wrong Stadium:
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
  // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::direct_forest(engine),
         "The direct held-Forest route was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell was not dominated by held Forest.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 0,
         "Field Blower was not dominated by held Forest.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.path_lock_removed,
         "Direct Forest did not remove the modeled Path lock.");
  expect(state.stadium == sim::Stadium::ForestOfVitality,
         "Direct Forest did not remain as the active Stadium.");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::FieldBlower) == 1,
         "Direct Forest unnecessarily spent Field Blower.");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::ChaoticSwell) == 1,
         "Direct Forest unnecessarily spent Chaotic Swell.");
  expect(!trace_contains(trace, "Field Blower discarded the modeled Path"),
         "The dominated Field Blower route still executed.");
  expect(trace_contains(trace, "Played Forest of Vitality"),
         "The trace did not record direct Forest replacement.");
  expect(trace_contains(trace, "Forest of Vitality allowed"),
         "The current-turn Pineco did not use Forest-enabled evolution.");
}

void test_direct_forest_remains_legal_through_item_lock() {
  std::mt19937_64 rng(306402);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_state(engine, direct_forest_state());

  // Forest is a Stadium, so Item lock does not prevent the direct replacement:
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(sim::EngineTestAccess::direct_forest(engine),
         "Item lock incorrectly disabled the held-Forest route.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 0,
         "Item lock made the dominated Field Blower route visible.");
}

void test_deferred_prized_forest_uses_field_blower_tapu_gladion() {
  std::mt19937_64 rng(306403);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, deferred_prized_forest_state(),
                                   true, true);

  // K1 proves Forest is prized and Gladion remains in deck. Path suppresses
  // Wonder Tag, so Field Blower is the legal pre-Forest unlock that preserves the
  // Stadium action; Tapu Lele-GX then searches Gladion and Gladion recovers Forest:
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
  // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "A prized Forest was misclassified as held.");
  expect(sim::EngineTestAccess::deferred_forest(engine),
         "The K1 deferred Forest bridge was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell preempted the deferred Forest bridge.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 1,
         "Field Blower was hidden from the deferred Forest bridge.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.stadium == sim::Stadium::ForestOfVitality,
         "Deferred Forest was not played after Gladion recovery.");
  expect(std::count(state.discard.begin(), state.discard.end(),
                    sim::Card::FieldBlower) == 1,
         "Deferred route did not spend Field Blower.");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::ChaoticSwell) == 1,
         "Deferred route unnecessarily spent Chaotic Swell.");
  expect(std::count(state.prizes.begin(), state.prizes.end(),
                    sim::Card::Gladion) == 1,
         "Gladion did not exchange itself into the Prize zone.");
  expect(trace_contains(trace, "Wonder Tag searched Gladion"),
         "The deferred route did not resolve Wonder Tag.");
  expect(trace_contains(trace, "Gladion exchanged itself for the known prized Forest"),
         "The deferred route did not recover Forest with Gladion.");
  expect(trace_contains(trace, "Played Forest of Vitality"),
         "The deferred route did not spend the preserved Stadium action on Forest.");
}

void test_first_turn_does_not_reserve_forest() {
  std::mt19937_64 rng(306404);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_state(engine, direct_forest_state(1, 1));

  // Forest's special evolution permission applies only after the first turn:
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "The first turn incorrectly reserved Forest for entry-turn evolution.");
}

void test_prior_turn_pineco_does_not_reserve_forest() {
  std::mt19937_64 rng(306405);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_state(engine, direct_forest_state(2, 1));

  // A prior-turn Pineco already has ordinary evolution timing, so Forest adds no
  // same-turn evolution value:
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1377
  // https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "A prior-turn Pineco incorrectly reserved Forest.");
}

void test_item_lock_blocks_only_deferred_field_blower_route() {
  std::mt19937_64 rng(306406);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_state(engine, deferred_prized_forest_state(),
                                   true, true);

  // The deferred route requires Field Blower, an Item. With Forest still prized,
  // Item lock blocks this pre-Forest unlock and leaves Swell as the available
  // generic Stadium replacement:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::deferred_forest(engine),
         "Item lock incorrectly admitted the Field Blower deferred route.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "Item lock incorrectly hid the Swell fallback.");
}

void test_swell_only_state_keeps_swell_fallback() {
  std::mt19937_64 rng(306407);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::State state = deferred_prized_forest_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::FieldBlower),
                   state.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(state), true, true);

  // Without held Forest or a legal Field Blower bridge, Chaotic Swell remains the
  // live Path replacement:
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://github.com/FlareZ123/pokemon-sims/issues/3064
  expect(!sim::EngineTestAccess::deferred_forest(engine),
         "A deferred route existed without Field Blower.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "The Swell-only fallback was incorrectly hidden.");
}

void test_complete_energy_axis_keeps_generic_lock_removal() {
  std::mt19937_64 rng(306408);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::State state = direct_forest_state();
  state.active->grass = 2;
  state.active->fire = 1;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::direct_forest(engine),
         "A complete Energy axis unnecessarily reserved Forest for Forretress.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "A complete Energy axis incorrectly hid the generic Swell route.");
}

}  // namespace

int main() {
  try {
    test_held_forest_directly_replaces_path();
    test_direct_forest_remains_legal_through_item_lock();
    test_deferred_prized_forest_uses_field_blower_tapu_gladion();
    test_first_turn_does_not_reserve_forest();
    test_prior_turn_pineco_does_not_reserve_forest();
    test_item_lock_blocks_only_deferred_field_blower_route();
    test_swell_only_state_keeps_swell_fallback();
    test_complete_energy_axis_keeps_generic_lock_removal();
    std::cout << "Issue 3064 refined Forest dominance tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
