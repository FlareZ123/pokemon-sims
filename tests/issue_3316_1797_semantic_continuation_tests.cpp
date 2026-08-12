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
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool bank(Engine& engine) {
    return engine.issue_3316_1797_bank_continuation();
  }
  static bool finish(Engine& engine) {
    return engine.issue_3316_1797_finish_continuation();
  }
  static State& state(Engine& engine) { return engine.state_; }
  static int finish_turn(const Engine& engine) {
    return engine.issue_1797_finish_turn_;
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(
    const sim::DciProfile dci = sim::DciProfile::StrictJit,
    const sim::LockMode locks = sim::LockMode::None,
    const bool going_first = true,
    const int max_turn = 5) {
  return sim::Scenario{"issue-3316-1797-semantic-continuation",
                       dci, locks, going_first, max_turn};
}

sim::State bank_state(const int turn = 2, const int entered_turn = 1) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, entered_turn, 1, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, entered_turn, 0, 0,
                   sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::StevensResolve,
  };
  state.deck = {
      sim::Card::Crispin,
      sim::Card::EarthenVessel,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::QuickBall,
  };
  state.discard = {
      sim::Card::HisuianHeavyBall,
      sim::Card::QuickBall,
      sim::Card::TateLiza,
  };
  state.prizes = {
      sim::Card::ProfessorTuro,
      sim::Card::Dragapult,
      sim::Card::MysteriousTreasure,
      sim::Card::PathToPeak,
      sim::Card::Guzma,
      sim::Card::Oricorio,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          const std::uint64_t seed = 3316)
      : scenario_value(std::move(selected_scenario)),
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void advance_to_finish(sim::Engine& engine) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  ++state.turn;
  state.supporter_used = false;
  state.manual_energy_used = false;
  state.retreat_used = false;
  state.stadium_used = false;
  state.turn_ended = false;
  state.discarded_this_turn.clear();
}

void strict_original_state_banks_and_finishes() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());

  // The public K1 state already contains the Quick Ball/Tapu provenance. Steven
  // banks Crispin, Vessel, and the Dragon, then the next turn evolves, uses Crispin,
  // and pays Vessel's discard with that Dragon on the ready turn.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3316
  expect(sim::EngineTestAccess::bank(fixture.engine),
         "Original public #1797 bank state was rejected");
  expect(sim::EngineTestAccess::finish_turn(fixture.engine) == 3,
         "Bank did not record the relative next turn");
  advance_to_finish(fixture.engine);
  expect(sim::EngineTestAccess::finish(fixture.engine),
         "Original public #1797 finish state was rejected");
}

void matchup_flex_has_same_ready_turn_semantics() {
  Fixture fixture{scenario(sim::DciProfile::MatchupFlexJit)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());

  // Both JIT profiles require the payload to enter discard during the ready turn:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  expect(sim::EngineTestAccess::bank(fixture.engine),
         "MatchupFlexJit was still rejected by the bank continuation");
  advance_to_finish(fixture.engine);
  expect(sim::EngineTestAccess::finish(fixture.engine),
         "MatchupFlexJit was still rejected by the finish continuation");
}

void going_second_equivalent_state_is_admitted() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());

  // Current Supporter legality and prior-turn evolution age govern this established
  // state; no printed continuation card depends on the original seat identity.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  expect(sim::EngineTestAccess::bank(fixture.engine),
         "Going-second equivalent bank state was rejected");
  advance_to_finish(fixture.engine);
  expect(sim::EngineTestAccess::finish(fixture.engine),
         "Going-second equivalent finish state was rejected");
}

void later_equivalent_turn_pair_is_relative() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true, 4)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state(3, 2));

  // Steven ends whichever legal bank turn is current; the continuation belongs to
  // the immediately following player turn rather than absolute T3.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3316
  expect(sim::EngineTestAccess::bank(fixture.engine),
         "Later equivalent bank state was rejected");
  expect(sim::EngineTestAccess::finish_turn(fixture.engine) == 4,
         "Later bank did not record current turn plus one");
  advance_to_finish(fixture.engine);
  expect(sim::EngineTestAccess::finish(fixture.engine),
         "Later equivalent finish state was rejected");
}

void rule_box_only_lock_preserves_trainer_continuation() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::FullRuleBoxAbility)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());

  // The connector Ability already resolved. Rule Box lock does not suppress these
  // Supporter/Item actions in the established continuation:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  expect(sim::EngineTestAccess::bank(fixture.engine),
         "Rule Box-only lock incorrectly rejected the Trainer continuation");
  advance_to_finish(fixture.engine);
  expect(sim::EngineTestAccess::finish(fixture.engine),
         "Rule Box-only lock incorrectly rejected the finish turn");
}

void no_discard_control_is_not_this_jit_route() {
  Fixture fixture{scenario(sim::DciProfile::NoDiscardControl)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "NoDiscardControl entered the same-ready-turn JIT continuation");
}

void projected_turn_two_item_lock_rejects_bank() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::TurnTwoItem)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state(1, 0));

  // The next turn requires Earthen Vessel, so scheduled T2 Item lock invalidates
  // the package before Steven consumes the current Supporter action.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Bank ignored projected turn-two Item lock");
}

void full_item_and_combined_locks_reject_bank() {
  for (const sim::LockMode lock :
       {sim::LockMode::FullItem, sim::LockMode::FullCombined}) {
    Fixture fixture{scenario(sim::DciProfile::StrictJit, lock)};
    sim::EngineTestAccess::set_state(fixture.engine, bank_state());
    expect(!sim::EngineTestAccess::bank(fixture.engine),
           "Item-dependent continuation crossed an Item lock");
  }
}

void supporter_lock_rejects_bank() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::FullSupporter)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Steven continuation crossed Supporter lock");
}

void k0_rejects_bank() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, bank_state(), false);
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Bank continuation read hidden deck/Prize identities at K0");
}

void evolution_age_rejects_bank() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, bank_state(2, 2));
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Bank accepted a Regidrago V played during the current turn");
}

void spent_attachment_rejects_bank() {
  Fixture fixture;
  sim::State state = bank_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Bank invented a second manual attachment");
}

void missing_vessel_rejects_bank() {
  Fixture fixture;
  sim::State state = bank_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::EarthenVessel), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Bank invented Earthen Vessel access");
}

void horizon_exhaustion_rejects_bank() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, bank_state(2, 1));
  expect(!sim::EngineTestAccess::bank(fixture.engine),
         "Bank exceeded the configured setup horizon");
}

void stale_or_missing_marker_rejects_finish() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, bank_state());
  expect(!sim::EngineTestAccess::finish(fixture.engine),
         "Finish ran without a prior bank marker");

  expect(sim::EngineTestAccess::bank(fixture.engine),
         "Precondition bank failed");
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  state.turn += 2;
  state.supporter_used = false;
  state.manual_energy_used = false;
  state.turn_ended = false;
  expect(!sim::EngineTestAccess::finish(fixture.engine),
         "Finish accepted a stale relative bank marker");
}

}  // namespace

int main() {
  try {
    strict_original_state_banks_and_finishes();
    matchup_flex_has_same_ready_turn_semantics();
    going_second_equivalent_state_is_admitted();
    later_equivalent_turn_pair_is_relative();
    rule_box_only_lock_preserves_trainer_continuation();
    no_discard_control_is_not_this_jit_route();
    projected_turn_two_item_lock_rejects_bank();
    full_item_and_combined_locks_reject_bank();
    supporter_lock_rejects_bank();
    k0_rejects_bank();
    evolution_age_rejects_bank();
    spent_attachment_rejects_bank();
    missing_vessel_rejects_bank();
    horizon_exhaustion_rejects_bank();
    stale_or_missing_marker_rejects_finish();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
