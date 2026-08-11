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
  static bool direct_forest(const Engine& engine) {
    return engine.issue_3057_direct_forest_over_path();
  }
  static bool deferred_forest(const Engine& engine) {
    return engine.issue_3057_deferred_forest_via_field_blower();
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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State direct_route_state(const int turn = 2,
                              const int pineco_entered_turn = 2) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco,
                                     pineco_entered_turn, 0, 0,
                                     sim::Tool::None});
  state.hand = {sim::Card::FieldBlower, sim::Card::ChaoticSwell,
                sim::Card::ForestOfVitality, sim::Card::ForretressEx};
  state.deck = {sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass};
  return state;
}

sim::State deferred_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 2, 0, 0,
                                     sim::Tool::None});
  state.hand = {
      sim::Card::FieldBlower,
      sim::Card::ChaoticSwell,
      sim::Card::SecretBox,
      sim::Card::ForretressEx,
      sim::Card::RegidragoVstar,
      sim::Card::Dragapult,
      sim::Card::Fire,
      sim::Card::ProfessorTuro,
      sim::Card::ProfessorBurnet,
      sim::Card::Guzma,
  };
  state.deck = {
      sim::Card::ForestOfVitality,
      sim::Card::MysteriousTreasure,
      sim::Card::Dawn,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
  };
  return state;
}

sim::Engine make_engine(const sim::LockMode locks, const sim::DciProfile dci,
                        std::mt19937_64& rng, sim::TraceLog* trace = nullptr) {
  const sim::Scenario scenario{"issue-3057", dci, locks, false, 3};
  return sim::Engine(scenario, sim::pineco_recipe(), rng, trace);
}

void test_held_forest_directly_replaces_path() {
  std::mt19937_64 rng(305701);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                   sim::DciProfile::StrictJit, rng, &trace);
  sim::EngineTestAccess::set_state(engine, direct_route_state());

  // Forest is already held, so it dominates both generic removal channels: its
  // Stadium play replaces Path and remains active for the current-turn Pineco
  // evolution. Field Blower and Chaotic Swell spend extra resources or the unique
  // Stadium action without adding setup progress.
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Core Stadium and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(sim::EngineTestAccess::direct_forest(engine),
         "Held Forest route was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell was not deferred for direct Forest.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 0,
         "Field Blower was not deferred for direct Forest.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.path_lock_removed &&
             state.stadium == sim::Stadium::ForestOfVitality,
         "Forest did not directly replace the modeled Path.");
  expect(contains(state.hand, sim::Card::FieldBlower) &&
             contains(state.hand, sim::Card::ChaoticSwell),
         "Direct Forest spent a dominated lock-removal card.");
  expect(trace_contains(trace, "Played Forest of Vitality"),
         "Direct Forest play was not traced.");
  expect(trace_contains(trace, "Forest of Vitality allowed"),
         "Current-turn Pineco did not use Forest evolution timing.");
  expect(!trace_contains(trace, "Field Blower discarded the modeled Path"),
         "Direct Forest unnecessarily spent Field Blower.");
}

void test_direct_forest_survives_item_lock() {
  std::mt19937_64 rng(305702);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined,
                                   sim::DciProfile::StrictJit, rng);
  sim::EngineTestAccess::set_state(engine, direct_route_state());

  // Forest is a Stadium, so the direct route remains legal while Items are locked.
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Core Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(sim::EngineTestAccess::direct_forest(engine),
         "Item lock incorrectly blocked direct Forest.");
  sim::EngineTestAccess::run_turn(engine);
  expect(sim::EngineTestAccess::state(engine).stadium ==
             sim::Stadium::ForestOfVitality,
         "Item lock prevented the legal direct Forest play.");
}

void test_k1_field_blower_preserves_stadium_for_secret_box_forest() {
  std::mt19937_64 rng(305703);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                   sim::DciProfile::MatchupFlexJit, rng, &trace);
  sim::EngineTestAccess::set_state(engine, deferred_route_state(), true);

  // K1 proves Forest is in deck and the post-Field-Blower Secret Box route is
  // payable. Field Blower may therefore remove Path without consuming the Stadium
  // play; Secret Box searches Forest, and the fixed-point retry revisits the
  // existing Forretress planner on the resulting public state.
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Core Item, search, Stadium, and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "Deferred fixture unexpectedly had a held Forest route.");
  expect(sim::EngineTestAccess::deferred_forest(engine),
         "K1 Secret Box deferred Forest route was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Deferred route did not preserve the Stadium action from Swell.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 1,
         "Deferred route hid the required Field Blower.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.path_lock_removed,
         "Deferred route did not remove the modeled Path.");
  expect(state.stadium == sim::Stadium::ForestOfVitality,
         "Secret Box route did not finish with Forest active.");
  expect(trace_contains(trace, "Field Blower discarded the modeled Path"),
         "Deferred route did not spend Field Blower first.");
  expect(trace_contains(trace, "Secret Box discarded three other cards"),
         "Deferred route did not resolve Secret Box.");
  expect(trace_contains(trace, "Played Forest of Vitality"),
         "Deferred route did not play the searched Forest.");
  expect(trace_contains(trace, "Forest of Vitality allowed"),
         "Deferred Forest did not enable current-turn Pineco evolution.");
}

void test_k0_does_not_assume_secret_box_forest_target() {
  std::mt19937_64 rng(305704);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                   sim::DciProfile::MatchupFlexJit, rng);
  sim::EngineTestAccess::set_state(engine, deferred_route_state(), false);

  // K0 cannot assert that Forest is in the hidden deck rather than Prizes, so the
  // deterministic deferred route is unavailable and Chaotic Swell stays visible.
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Secret Box / Forest: https://api.pokemontcg.io/v2/cards/sv6-163 https://api.pokemontcg.io/v2/cards/me1-117
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(!sim::EngineTestAccess::deferred_forest(engine),
         "K0 inferred the hidden Forest target.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "K0 incorrectly hid Chaotic Swell.");
}

void test_item_lock_disables_deferred_field_blower_route() {
  std::mt19937_64 rng(305705);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined,
                                   sim::DciProfile::MatchupFlexJit, rng);
  sim::EngineTestAccess::set_state(engine, deferred_route_state(), true);

  // Field Blower is an Item, so the deferred line is illegal under Item lock.
  // Chaotic Swell remains the legal generic Stadium channel because Forest is not
  // yet held in this fixture.
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Core Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(!sim::EngineTestAccess::deferred_forest(engine),
         "Item lock admitted the Field Blower route.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "Item lock incorrectly hid Chaotic Swell.");
}

void test_first_turn_and_normal_evolution_do_not_reserve_forest() {
  std::mt19937_64 rng(305706);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                   sim::DciProfile::StrictJit, rng);

  sim::EngineTestAccess::set_state(engine, direct_route_state(1, 1));
  // Forest's entry-turn evolution permission applies only after the player's first
  // turn: https://api.pokemontcg.io/v2/cards/me1-117
  // Core evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "First turn incorrectly reserved Forest for entry-turn evolution.");

  sim::EngineTestAccess::set_state(engine, direct_route_state(2, 1));
  // Prior-turn Pineco already satisfies ordinary evolution timing, so Forest adds
  // no timing axis and generic Path removal retains its normal priority.
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "Prior-turn Pineco unnecessarily reserved Forest.");
}

void test_missing_live_route_keeps_chaotic_swell_channel() {
  std::mt19937_64 rng(305707);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                   sim::DciProfile::StrictJit, rng);
  sim::State state = direct_route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::ForretressEx),
                   state.hand.end());
  state.deck = {sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // With no held or known deck-resident Forretress ex, Forest cannot supply a live
  // same-turn evolution route. Chaotic Swell therefore stays available.
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "Missing Forretress still reserved Forest.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "Missing live route hid Chaotic Swell.");
}

void test_complete_energy_axis_keeps_generic_removal() {
  std::mt19937_64 rng(305708);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility,
                                   sim::DciProfile::StrictJit, rng);
  sim::State state = direct_route_state();
  state.active->grass = 2;
  state.active->fire = 1;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Exploding Energy no longer advances setup once GGF is complete, so Forest's
  // entry-turn evolution route does not dominate generic lock removal.
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(!sim::EngineTestAccess::direct_forest(engine),
         "Complete Energy axis unnecessarily reserved Forest.");
}

}  // namespace

int main() {
  try {
    test_held_forest_directly_replaces_path();
    test_direct_forest_survives_item_lock();
    test_k1_field_blower_preserves_stadium_for_secret_box_forest();
    test_k0_does_not_assume_secret_box_forest_target();
    test_item_lock_disables_deferred_field_blower_route();
    test_first_turn_and_normal_evolution_do_not_reserve_forest();
    test_missing_live_route_keeps_chaotic_swell_channel();
    test_complete_energy_axis_keeps_generic_removal();
    std::cout << "Issue 3057 refined Forest ordering tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
