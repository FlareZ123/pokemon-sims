#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static std::optional<Card> redundant_payload_cost(const Engine& engine) {
    return engine.issue_2323_redundant_payload_cost();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  Fixture(const sim::DciProfile dci,
          const sim::LockMode locks = sim::LockMode::None,
          const int max_turn = 5)
      : scenario{"issue-3029/exact", dci, locks, false, max_turn},
        recipe{sim::baseline_recipe()},
        rng{3029},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State redundant_payload_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0,
                              sim::Tool::None, 0};
  state.hand = {
      sim::Card::MysteriousTreasure,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
  };
  return state;
}

std::optional<sim::Card> projected_cost(
    const sim::DciProfile dci,
    sim::State state = redundant_payload_state(),
    const sim::LockMode locks = sim::LockMode::None,
    const int max_turn = 5) {
  Fixture fixture{dci, locks, max_turn};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  return sim::EngineTestAccess::redundant_payload_cost(fixture.engine);
}

void test_both_same_turn_jit_profiles_admit_identical_route() {
  const std::optional<sim::Card> strict = projected_cost(sim::DciProfile::StrictJit);
  const std::optional<sim::Card> flex = projected_cost(sim::DciProfile::MatchupFlexJit);

  // Two distinct held Dragon payloads make one route-redundant for Mysterious
  // Treasure while the other remains protected for the eventual ready turn. The
  // repository assigns StrictJit and MatchupFlexJit the same ready-turn payload
  // requirement, so the identical observable K0 state must expose the same cost:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Advanced Item, Supporter, attachment, and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K0/K1 and same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route / confirmed generalization: https://github.com/FlareZ123/pokemon-sims/issues/2323 https://github.com/FlareZ123/pokemon-sims/issues/3029
  expect(strict.has_value(), "StrictJit lost the established redundant-payload route.");
  expect(flex.has_value(), "MatchupFlexJit was blocked by the historical profile identity.");
  expect(*strict == *flex,
         "The same physical route selected different redundant Dragon costs by JIT profile.");
}

void test_non_jit_profile_remains_excluded() {
  // NoDiscardControl does not carry the repository's ready-turn payload timing, so
  // the issue-2323 JIT-specific redundant-payload fallback must stay unavailable:
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/3029
  expect(!projected_cost(sim::DciProfile::NoDiscardControl).has_value(),
         "The JIT-specific Treasure fallback leaked into NoDiscardControl.");
}

void test_only_one_payload_remains_protected() {
  sim::State state = redundant_payload_state();
  state.hand.pop_back();

  // Mysterious Treasure requires a discard cost, and one singleton payload remains
  // protected by the repository's same-turn JIT DCI policy:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  expect(!projected_cost(sim::DciProfile::MatchupFlexJit, std::move(state)).has_value(),
         "A singleton Dragon payload became a redundant Treasure cost.");
}

void test_already_held_regidrago_disables_search_route() {
  sim::State state = redundant_payload_state();
  state.hand.push_back(sim::Card::RegidragoV);

  // The fallback exists to search Regidrago V and start its evolution timer. A
  // held Regidrago V already satisfies that axis, so paying a payload is dominated:
  // Mysterious Treasure / Regidrago V: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12-135
  // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(!projected_cost(sim::DciProfile::MatchupFlexJit, std::move(state)).has_value(),
         "The redundant-payload fallback remained live after the Regidrago axis was held.");
}

void test_crispin_or_energy_progression_is_required() {
  sim::State no_crispin = redundant_payload_state();
  const auto crispin = std::find(no_crispin.hand.begin(), no_crispin.hand.end(),
                                 sim::Card::Crispin);
  no_crispin.hand.erase(crispin);
  expect(!projected_cost(sim::DciProfile::MatchupFlexJit,
                         std::move(no_crispin)).has_value(),
         "Treasure spent a payload without the held Crispin progression.");

  sim::State no_grass = redundant_payload_state();
  const auto grass = std::find(no_grass.hand.begin(), no_grass.hand.end(),
                               sim::Card::Grass);
  no_grass.hand.erase(grass);
  // Crispin and the held/public Basic-Energy axis are part of the observable T1
  // progression proof; absent either one, the payload discard is not justified:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Advanced attachment/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  expect(!projected_cost(sim::DciProfile::MatchupFlexJit,
                         std::move(no_grass)).has_value(),
         "Treasure spent a payload without the held Grass progression.");
}

void test_item_lock_and_short_horizon_disable_route() {
  // Mysterious Treasure is an Item, so full Item lock makes the T1 route illegal.
  // A horizon before T3 also removes the modeled payoff that justified spending a
  // redundant payload to start the evolution timer:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Item/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/3029
  expect(!projected_cost(sim::DciProfile::MatchupFlexJit,
                         redundant_payload_state(),
                         sim::LockMode::FullItem).has_value(),
         "The Treasure fallback remained live under Item lock.");
  expect(!projected_cost(sim::DciProfile::MatchupFlexJit,
                         redundant_payload_state(),
                         sim::LockMode::None, 2).has_value(),
         "The Treasure fallback remained live without a T3 payoff horizon.");
}

}  // namespace

int main() {
  try {
    test_both_same_turn_jit_profiles_admit_identical_route();
    test_non_jit_profile_remains_excluded();
    test_only_one_payload_remains_protected();
    test_already_held_regidrago_disables_search_route();
    test_crispin_or_energy_progression_is_required();
    test_item_lock_and_short_horizon_disable_route();
    std::cout << "Issue 3029 shared-JIT redundant-payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
