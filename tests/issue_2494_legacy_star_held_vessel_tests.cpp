#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool delayed_vessel_route(Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
  static bool recover_discard_to_hand(Engine& engine, const Card card) {
    return engine.recover_discard_to_hand(card);
  }
};

}  // namespace sim

namespace {

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

struct Fixture {
  sim::Scenario scenario{"issue-2494", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2494};
  sim::Engine engine{scenario, recipe, rng};
};

void seed_delayed_route(sim::Engine& engine, const bool hold_vessel,
                        const bool discard_vessel) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite};
  if (hold_vessel) state.hand.push_back(sim::Card::EarthenVessel);
  if (discard_vessel) state.discard.push_back(sim::Card::EarthenVessel);
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_held_vessel_is_not_a_discard_recovery_target() {
  Fixture fixture;
  seed_delayed_route(fixture.engine, true, false);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);

  if (!sim::EngineTestAccess::delayed_vessel_route(fixture.engine)) {
    throw std::runtime_error("Held Vessel should satisfy the delayed route predicate.");
  }
  // Legacy Star chooses cards from the discard pile, so a copy already in hand is
  // unavailable to its recovery selector: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel identity: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed zone-selection bug: https://github.com/FlareZ123/pokemon-sims/issues/2494
  if (sim::EngineTestAccess::recover_discard_to_hand(
          fixture.engine, sim::Card::EarthenVessel)) {
    throw std::runtime_error("A held-only Vessel cannot be recovered from discard.");
  }
  if (count_card(state.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(state.discard, sim::Card::EarthenVessel) != 0) {
    throw std::runtime_error("Held-only Vessel must remain in its original zone.");
  }
}

void test_discard_vessel_remains_a_legal_recovery_target() {
  Fixture fixture;
  seed_delayed_route(fixture.engine, false, true);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);

  if (!sim::EngineTestAccess::delayed_vessel_route(fixture.engine)) {
    throw std::runtime_error("Discard Vessel should satisfy the delayed route predicate.");
  }
  // Legacy Star may recover a physical discard copy: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Existing delayed-route specification: https://github.com/FlareZ123/pokemon-sims/issues/1844
  if (!sim::EngineTestAccess::recover_discard_to_hand(
          fixture.engine, sim::Card::EarthenVessel)) {
    throw std::runtime_error("A discard Vessel should remain recoverable.");
  }
  if (count_card(state.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(state.discard, sim::Card::EarthenVessel) != 0) {
    throw std::runtime_error("Discard Vessel should move to hand exactly once.");
  }
}

void test_held_and_discard_copies_are_distinct_physical_cards() {
  Fixture fixture;
  seed_delayed_route(fixture.engine, true, true);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);

  if (!sim::EngineTestAccess::delayed_vessel_route(fixture.engine)) {
    throw std::runtime_error("Held-plus-discard Vessel should satisfy the delayed route predicate.");
  }
  // The production #2494 selector preserves its Legacy Star recovery slot when one
  // Vessel is already held; this control proves the zones contain two distinct cards:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  if (count_card(state.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(state.discard, sim::Card::EarthenVessel) != 1) {
    throw std::runtime_error("Held and discard Vessel copies must remain distinct.");
  }
}

void adjust(sim::DeckRecipe& recipe, const sim::Card card, const int delta) {
  const auto found = std::find_if(
      recipe.begin(), recipe.end(), [card](const auto& entry) {
        return entry.first == card;
      });
  if (found == recipe.end()) {
    recipe.push_back({card, delta});
    return;
  }
  found->second += delta;
  if (found->second == 0) recipe.erase(found);
}

void test_reported_production_sweep_no_longer_throws() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  adjust(recipe, sim::Card::ErikasInvitation, -1);
  adjust(recipe, sim::Card::PokemonCommunication, 1);
  std::string error;
  if (!sim::validate_recipe({"issue-2494-reproduction", recipe}, &error)) {
    throw std::runtime_error("Reproduction recipe became invalid: " + error);
  }

  const auto found = sim::scenario_by_label("strict-jit/go-second");
  if (!found) throw std::runtime_error("strict-jit/go-second scenario disappeared.");
  sim::Scenario scenario = *found;
  scenario.max_turn = 4;

  // Reproduce the exact 10,000-trial common-random-number stream that exposed #2494
  // in PR #2492. Before the production fix, one trial throws
  // "Legacy Star delayed Earthen Vessel target disappeared". A held card is outside
  // Legacy Star's selectable discard-pile zone: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Reproduction harness: https://github.com/FlareZ123/pokemon-sims/pull/2492
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2494
  constexpr std::uint64_t base_seed = 20260809ULL;
  constexpr std::uint64_t trial_stride = 104729ULL;
  constexpr std::uint64_t scenario_stride = 1000000007ULL;
  constexpr std::uint64_t strict_go_second_index = 1ULL;
  constexpr std::uint64_t trials = 10000ULL;
  for (std::uint64_t trial = 0; trial < trials; ++trial) {
    const std::uint64_t trial_seed = base_seed +
        scenario_stride * strict_go_second_index + trial_stride * trial;
    std::mt19937_64 rng(trial_seed);
    sim::Engine engine(scenario, recipe, rng);
    (void)engine.run();
  }
}

}  // namespace

int main() {
  try {
    test_held_vessel_is_not_a_discard_recovery_target();
    test_discard_vessel_remains_a_legal_recovery_target();
    test_held_and_discard_copies_are_distinct_physical_cards();
    test_reported_production_sweep_no_longer_throws();
    std::cout << "issue 2494 Legacy Star held Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
