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

sim::State live_route_state(const int pineco_entered_turn = 2) {
  sim::State state;
  state.turn = 2;
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

sim::Engine make_engine(const sim::LockMode locks, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::Scenario scenario{"issue-3057", sim::DciProfile::StrictJit,
                               locks, false, 3};
  return sim::Engine(scenario, sim::pineco_recipe(), rng, trace);
}

void test_field_blower_preserves_forest_for_current_turn_pineco() {
  std::mt19937_64 rng(305701);
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng, &trace);
  sim::EngineTestAccess::set_state(engine, live_route_state());

  // Field Blower removes Path as an Item, preserving the one Stadium play for
  // Forest of Vitality's entry-turn Grass evolution permission. Chaotic Swell
  // must remain held because spending the Stadium action on it blocks this ALS:
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(sim::EngineTestAccess::preserve_forest_action(engine),
         "The live Field Blower -> Forest route was not recognized.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 0,
         "Chaotic Swell was not deferred for the live Forest route.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::FieldBlower) == 1,
         "Field Blower was hidden with Chaotic Swell.");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.path_lock_removed,
         "Field Blower did not remove the modeled Path lock.");
  expect(state.stadium == sim::Stadium::ForestOfVitality,
         "Forest of Vitality did not receive the preserved Stadium action.");
  expect(std::count(state.discard.begin(), state.discard.end(),
                    sim::Card::FieldBlower) == 1,
         "Field Blower was not spent on the Path-removal channel.");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::ChaoticSwell) == 1,
         "The dominated Chaotic Swell play was not preserved in hand.");
  expect(trace_contains(trace, "Field Blower discarded the modeled Path"),
         "The trace did not record the Field Blower Path removal.");
  expect(trace_contains(trace, "Played Forest of Vitality"),
         "The trace did not record the preserved Forest play.");
  expect(trace_contains(trace, "Forest of Vitality allowed"),
         "The current-turn Pineco did not use Forest-enabled evolution.");
}

void test_item_lock_keeps_chaotic_swell_channel() {
  std::mt19937_64 rng(305702);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_state(engine, live_route_state());

  // Item lock makes Field Blower illegal, so Chaotic Swell remains the live
  // Stadium channel for Path removal: https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "Item lock incorrectly selected the Field Blower route.");
  expect(sim::EngineTestAccess::late_hand_count(engine,
                                                sim::Card::ChaoticSwell) == 1,
         "Item lock incorrectly hid Chaotic Swell.");
}

void test_prior_turn_pineco_does_not_reserve_forest() {
  std::mt19937_64 rng(305703);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_state(engine, live_route_state(1));

  // A prior-turn Pineco already has ordinary evolution timing, so Forest adds no
  // setup value and must not displace Chaotic Swell:
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1377
  // https://github.com/FlareZ123/pokemon-sims/issues/3057
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "A prior-turn Pineco incorrectly reserved the Stadium action for Forest.");
}

void test_missing_route_piece_keeps_swell_priority() {
  std::mt19937_64 rng(305704);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);

  sim::State no_forest = live_route_state();
  no_forest.hand.erase(std::remove(no_forest.hand.begin(), no_forest.hand.end(),
                                   sim::Card::ForestOfVitality),
                       no_forest.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(no_forest));
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "The selector reserved a missing Forest of Vitality.");

  sim::State no_blower = live_route_state();
  no_blower.hand.erase(std::remove(no_blower.hand.begin(), no_blower.hand.end(),
                                   sim::Card::FieldBlower),
                       no_blower.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(no_blower));
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "The selector reserved Forest without Field Blower.");

  sim::State no_grass = live_route_state();
  no_grass.deck.clear();
  sim::EngineTestAccess::set_state(engine, std::move(no_grass), true);
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "The selector reserved Forest when Exploding Energy has no known Grass target.");
}

void test_complete_energy_axis_keeps_swell_priority() {
  std::mt19937_64 rng(305705);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::State state = live_route_state();
  state.active->grass = 2;
  state.active->fire = 1;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::preserve_forest_action(engine),
         "A complete Energy axis unnecessarily reserved Forest for Exploding Energy.");
}

}  // namespace

int main() {
  try {
    test_field_blower_preserves_forest_for_current_turn_pineco();
    test_item_lock_keeps_chaotic_swell_channel();
    test_prior_turn_pineco_does_not_reserve_forest();
    test_missing_route_piece_keeps_swell_priority();
    test_complete_energy_axis_keeps_swell_priority();
    std::cout << "Issue 3057 Field Blower / Forest ordering tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
