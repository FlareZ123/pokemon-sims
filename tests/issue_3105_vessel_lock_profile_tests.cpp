#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State ready_state() {
  // Reuse issue #1868's established complete public state so this regression
  // changes one dimension only: the lock profile.
  // Existing route contract: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1868_vessel_payload_dci_tests.cpp
  // Confirmed lock-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/3105
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::Guzma, sim::Card::DialgaGX,
                sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass, sim::Card::Grass,
                sim::Card::QuickBall};
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::deck_by_id("regidrago-shell")->recipe};
  std::mt19937_64 rng{3105};
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks)
      : scenario{"issue-3105", sim::DciProfile::MatchupFlexJit,
                 locks, true, 5},
        engine{scenario, recipe, rng} {}
};

bool discarded_payload(const sim::Engine& engine) {
  const sim::State& state = sim::EngineTestAccess::state(engine);
  return std::find(state.discarded_this_turn.begin(),
                   state.discarded_this_turn.end(), sim::Card::DialgaGX) !=
         state.discarded_this_turn.end();
}

void rule_box_ability_lock_keeps_vessel_legal() {
  Fixture fixture{sim::LockMode::FullRuleBoxAbility};
  sim::EngineTestAccess::set_state(fixture.engine, ready_state(), true);

  // Rule Box Ability lock does not prohibit Trainer Items, deck search, discard,
  // or the manual Basic Energy attachment used by this Earthen Vessel route:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item/search/discard/attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Scenario-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Established route fixture: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1868_vessel_payload_dci_tests.cpp
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3105
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "Rule Box Ability lock rejected the legal Earthen Vessel finish");
  expect(discarded_payload(fixture.engine),
         "current-turn Dragon payload was not discarded");
}

void item_lock_still_blocks_vessel() {
  Fixture fixture{sim::LockMode::FullItem};
  sim::EngineTestAccess::set_state(fixture.engine, ready_state(), true);

  // Earthen Vessel is an Item, so an active Item lock still blocks the route:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/issues/3105
  sim::EngineTestAccess::play_vessel(fixture.engine);
  expect(!discarded_payload(fixture.engine),
         "Item lock did not block Earthen Vessel");
}

void k0_still_blocks_vessel() {
  Fixture fixture{sim::LockMode::FullRuleBoxAbility};
  sim::EngineTestAccess::set_state(fixture.engine, ready_state(), false);

  // The route is K1-only because it depends on a proven searchable Basic Energy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/3105
  sim::EngineTestAccess::play_vessel(fixture.engine);
  expect(!discarded_payload(fixture.engine),
         "K0 state was allowed to use the K1 Vessel finish");
}

void spent_manual_attachment_still_blocks_vessel() {
  Fixture fixture{sim::LockMode::FullRuleBoxAbility};
  sim::State state = ready_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true);

  // The searched Basic only completes Apex through the still-unused manual
  // attachment, so spending that attachment invalidates the route:
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/3105
  sim::EngineTestAccess::play_vessel(fixture.engine);
  expect(!discarded_payload(fixture.engine),
         "spent manual attachment was ignored");
}

}  // namespace

int main() {
  try {
    rule_box_ability_lock_keeps_vessel_legal();
    item_lock_still_blocks_vessel();
    k0_still_blocks_vessel();
    spent_manual_attachment_still_blocks_vessel();
    std::cout << "Issue 3105 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
