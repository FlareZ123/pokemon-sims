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
  static bool quick_route(const Engine& engine) {
    return engine.issue_3458_1797_quick_ball_tapu_steven_route_available();
  }
  static bool wonder_route(const Engine& engine) {
    return engine.issue_3458_1797_wonder_tag_steven_route_available();
  }
  static std::optional<int> projected_finish(const Engine& engine) {
    return engine.issue_3458_1797_projected_finish_turn();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static Card choose_supporter(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
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
    const int max_turn = 3) {
  return sim::Scenario{"issue-3458-1797-semantic-connector",
                       dci, locks, going_first, max_turn};
}

sim::State deferred_t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::TateLiza,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::StevensResolve,
      sim::Card::Crispin,
      sim::Card::EarthenVessel,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
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

sim::State current_bank_state(const int turn) {
  sim::State state = deferred_t1_state();
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 0,
                              sim::Tool::None};
  return state;
}

sim::State wonder_state(const int turn) {
  sim::State state = current_bank_state(turn);
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::QuickBall), state.hand.end());
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::TateLiza), state.hand.end());
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::TapuLeleGX), state.deck.end());
  state.bench.push_back(
      sim::Pokemon{sim::Card::TapuLeleGX, turn, 0, 0, sim::Tool::None});
  state.discard = {sim::Card::QuickBall, sim::Card::TateLiza};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          const std::uint64_t seed = 3458)
      : scenario_value(std::move(selected_scenario)),
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void first_turn_bank_accepts_k1_without_heavy_ball_provenance() {
  Fixture fixture;
  sim::State state = deferred_t1_state();
  expect(std::find(state.discard.begin(), state.discard.end(),
                   sim::Card::HisuianHeavyBall) == state.discard.end(),
         "Fixture accidentally retained Hisuian Heavy Ball provenance");
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // K1 is the knowledge state regardless of which legal inspection established it.
  // The first-player T1 Supporter restriction banks Steven until T2, preserving the
  // original #1797 relative T3 finish.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Advanced first-turn and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Original route / connector fix: https://github.com/FlareZ123/pokemon-sims/issues/1797 https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "K1 route still required Heavy Ball provenance");
  const auto finish = sim::EngineTestAccess::projected_finish(fixture.engine);
  expect(finish && *finish == 3,
         "Going-first T1 bank did not project the relative T3 finish");
}

void going_second_current_supporter_bank_is_accepted() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(2));

  // A current-Supporter bank is legal once the prior-turn Regidrago V already has
  // one Grass and the second manual attachment is available. #3316 attaches that
  // held Grass before Steven ends T2, then finishes on T3.
  // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Semantic continuation: https://github.com/FlareZ123/pokemon-sims/issues/3316
  // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "Going-second current-Supporter connector was rejected");
  const auto finish = sim::EngineTestAccess::projected_finish(fixture.engine);
  expect(finish && *finish == 3,
         "Going-second current bank did not project current turn plus one");
}

void later_equivalent_turn_is_relative() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true, 5)};
  sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(4));
  const auto finish = sim::EngineTestAccess::projected_finish(fixture.engine);

  // Once the physical #3316 bank state exists, absolute T1/T3 coordinates are not
  // part of card legality or the repository route policy.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "Later equivalent connector state was rejected");
  expect(finish && *finish == 5,
         "Later connector did not project current turn plus one");
}

void impossible_zero_energy_current_supporter_projection_is_rejected() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 2)};
  sim::State state = deferred_t1_state();
  state.turn = 1;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Playing Steven immediately ends the turn. From zero attached Energy, the next
  // turn can supply only the legal attachment throughput modeled by the continuation,
  // so this state is not the proven #3316 bank and must not promise a T2 GGF finish.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced attachment and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Semantic continuation: https://github.com/FlareZ123/pokemon-sims/issues/3316
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Zero-Energy current-Supporter state projected an impossible finish");
}

void matchup_flex_uses_same_ready_turn_contract() {
  Fixture fixture{scenario(sim::DciProfile::MatchupFlexJit,
                           sim::LockMode::None, false, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(2));

  // Both JIT profiles use the same ready-turn payload timing, and Earthen Vessel
  // supplies the Dragon discard on that ready turn.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "MatchupFlexJit was rejected by the connector contract");
}

void live_quick_ball_preserves_package_and_fetches_tapu() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(2));

  // The confirmed DCI payment spends Tate & Liza while preserving VSTAR and both
  // held Grass, then Quick Ball searches Tapu Lele-GX.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::play_quick_ball(fixture.engine),
         "Live Quick Ball did not execute the semantic connector");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(std::count(state.discard.begin(), state.discard.end(),
                    sim::Card::TateLiza) == 1,
         "Quick Ball did not pay the approved Tate & Liza cost");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::RegidragoVstar) == 1,
         "Quick Ball spent the protected VSTAR package");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::Grass) == 2,
         "Quick Ball spent a protected Grass attachment");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::TapuLeleGX) == 1,
         "Quick Ball did not search Tapu Lele-GX");
}

void wonder_tag_selector_reuses_same_contract() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, wonder_state(2));

  // Post-Quick-Ball state differs only by the completed Item transaction. Wonder Tag
  // therefore chooses Steven from the same route contract.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::wonder_route(fixture.engine),
         "Post-Quick-Ball route diverged from the shared contract");
  expect(sim::EngineTestAccess::choose_supporter(fixture.engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not select Steven's Resolve");
}

void k0_is_rejected() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, deferred_t1_state(), false);
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Connector read fixed-list deck or Prize identities at K0");
}

void current_and_projected_locks_are_rejected() {
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::FullItem, false, 3)};
    sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(2));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Quick Ball connector crossed current Item lock");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::FullRuleBoxAbility, false, 3)};
    sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(2));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector ignored current Rule Box Ability lock");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::TurnTwoItem, true, 3)};
    sim::EngineTestAccess::set_state(fixture.engine, deferred_t1_state());
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector ignored persistent projected Item lock on its T3 finish");
  }
}

void supporter_lock_and_spent_slot_are_rejected() {
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::FullSupporter, true, 3)};
    sim::EngineTestAccess::set_state(fixture.engine, deferred_t1_state());
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector crossed full Supporter lock");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 3)};
    sim::State state = current_bank_state(2);
    state.supporter_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector ignored current Supporter contention");
  }
}

void bench_dci_resource_and_horizon_controls_are_rejected() {
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 3)};
    sim::State state = current_bank_state(2);
    for (int index = 0; index < 5; ++index) {
      state.bench.push_back(
          sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None});
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Quick Ball route invented Bench space for Tapu Lele-GX");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 3)};
    sim::State state = current_bank_state(2);
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::TateLiza), state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Quick Ball route invented an approved dynamic-DCI cost");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 3)};
    sim::State state = current_bank_state(2);
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::EarthenVessel), state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector invented Earthen Vessel access");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 2)};
    sim::EngineTestAccess::set_state(fixture.engine, deferred_t1_state());
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Going-first bank exceeded the configured T2 horizon");
  }
}

void no_discard_control_is_rejected() {
  Fixture fixture{scenario(sim::DciProfile::NoDiscardControl,
                           sim::LockMode::None, false, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, current_bank_state(2));
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "NoDiscardControl entered the same-ready-turn JIT connector");
}

}  // namespace

int main() {
  try {
    first_turn_bank_accepts_k1_without_heavy_ball_provenance();
    going_second_current_supporter_bank_is_accepted();
    later_equivalent_turn_is_relative();
    impossible_zero_energy_current_supporter_projection_is_rejected();
    matchup_flex_uses_same_ready_turn_contract();
    live_quick_ball_preserves_package_and_fetches_tapu();
    wonder_tag_selector_reuses_same_contract();
    k0_is_rejected();
    current_and_projected_locks_are_rejected();
    supporter_lock_and_spent_slot_are_rejected();
    bench_dci_resource_and_horizon_controls_are_rejected();
    no_discard_control_is_rejected();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
