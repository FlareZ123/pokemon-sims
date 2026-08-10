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
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1872_treasure_vessel_payload_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1872_treasure_vessel_payload_route();
  }
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

sim::Scenario jit(const sim::DciProfile profile, const bool going_first,
                  const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1872", profile, lock, going_first, 5};
}

sim::Scenario flex(const bool going_first,
                   const sim::LockMode lock = sim::LockMode::None) {
  return jit(sim::DciProfile::MatchupFlexJit, going_first, lock);
}

sim::State base_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 2, 1, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::ChaoticSwell, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                   prizes_revealed);
  return engine;
}

void test_exact_route_both_turn_orders_and_jit_profiles() {
  std::mt19937_64 rng(1872);
  for (const sim::DciProfile profile : {sim::DciProfile::StrictJit,
                                        sim::DciProfile::MatchupFlexJit}) {
    for (const bool going_first : {true, false}) {
      sim::Engine engine = make_engine(jit(profile, going_first), rng, base_state());
      // Both JIT profiles require the Dragon payload on the ready turn: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Mysterious Treasure, Earthen Vessel, and Apex Dragon: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12-136
      // Confirmed profile-overfitting bug: https://github.com/FlareZ123/pokemon-sims/issues/2742
      expect(sim::EngineTestAccess::route_available(engine),
             "The exact issue-1872 route was unavailable for a JIT profile/turn order.");
      expect(sim::EngineTestAccess::complete_route(engine),
             "The exact issue-1872 route did not complete.");
      expect(engine.state().active->grass == 2 && engine.state().active->fire == 1,
             "The issue-1872 route did not complete GGF.");
      expect(std::find(engine.state().discarded_this_turn.begin(),
                       engine.state().discarded_this_turn.end(),
                       sim::Card::MegaDragonite) !=
                 engine.state().discarded_this_turn.end(),
             "Earthen Vessel did not discard the searched Dragon this turn.");
      expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                       sim::Card::BrilliantBlender) != engine.state().hand.end(),
             "The deterministic route replayed Brilliant Blender.");
    }
  }

  // No-discard-control has different payload timing from the same-turn JIT profiles: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed profile-overfitting bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/2742
  sim::Engine no_control = make_engine(
      jit(sim::DciProfile::NoDiscardControl, true), rng, base_state());
  expect(!sim::EngineTestAccess::route_available(no_control),
         "No-discard-control admitted the JIT-only Treasure-Vessel route.");
}

void test_k1_lock_and_resource_boundaries() {
  std::mt19937_64 rng(453);
  const sim::Scenario no_lock = flex(true);
  const sim::Scenario item_lock = flex(true, sim::LockMode::FullItem);

  // A legal full-Prize inspection establishes K1 for exact composition checks,
  // while K0 and Item lock must reject this deterministic two-Item route:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Official Prize, Item, search, discard, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1872
  sim::Engine prize_k1 = make_engine(no_lock, rng, base_state(), false, true);
  expect(sim::EngineTestAccess::route_available(prize_k1),
         "Full Prize inspection K1 did not admit the route.");
  sim::Engine k0 = make_engine(no_lock, rng, base_state(), false, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the deterministic two-Item route.");
  sim::Engine locked = make_engine(item_lock, rng, base_state());
  expect(!sim::EngineTestAccess::route_available(locked),
         "Item lock admitted the Treasure-Vessel route.");

  sim::State no_stadium = base_state();
  no_stadium.hand.erase(std::find(no_stadium.hand.begin(), no_stadium.hand.end(),
                                  sim::Card::ChaoticSwell));
  sim::Engine missing_cost = make_engine(no_lock, rng, std::move(no_stadium));
  expect(!sim::EngineTestAccess::route_available(missing_cost),
         "The route was admitted without the Stadium cost.");

  sim::State no_grass = base_state();
  no_grass.deck.erase(std::remove(no_grass.deck.begin(), no_grass.deck.end(),
                                  sim::Card::Grass), no_grass.deck.end());
  sim::Engine missing_energy = make_engine(no_lock, rng, std::move(no_grass));
  expect(!sim::EngineTestAccess::route_available(missing_energy),
         "The route was admitted without searchable Grass Energy.");
}

void expect_registered_t4_route(const char* label, const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered issue-1872 deck is unavailable.");
  const auto scenario = sim::scenario_by_label(label);
  expect(scenario.has_value(), "A registered issue-1872 scenario is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  // These witnesses reach the exact T4 Treasure-Vessel state under their profile's earlier DCI decisions. The profile may make different earlier discard choices even though both JIT profiles share ready-turn payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Mysterious Treasure and Earthen Vessel: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed profile-overfitting bug: https://github.com/FlareZ123/pokemon-sims/issues/2742
  expect(outcome.first_ready_turn == 4,
         "A registered issue-1872 witness did not reach T4.");
  expect(trace_contains(trace, "Mysterious Treasure issue-1872") &&
             trace_contains(trace, "Earthen Vessel issue-1872") &&
             trace_contains(trace, "T4 | READY"),
         "A registered issue-1872 witness did not execute the complete T4 route.");
}

void test_registered_profile_witnesses_reach_t4() {
  expect_registered_t4_route("strict-jit/go-first", 605);
  expect_registered_t4_route("matchup-flex-jit/go-first", 453);
  expect_registered_t4_route("matchup-flex-jit/go-second", 453);
}
}  // namespace

int main() {
  try {
    test_exact_route_both_turn_orders_and_jit_profiles();
    test_k1_lock_and_resource_boundaries();
    test_registered_profile_witnesses_reach_t4();
    std::cout << "Issue 1872 Treasure-Vessel payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
