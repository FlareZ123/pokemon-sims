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

sim::State quick_state(const int turn = 1) {
  sim::State state;
  state.turn = turn;
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

sim::State wonder_state(const int turn = 1) {
  sim::State state = quick_state(turn);
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
  sim::State state = quick_state();
  expect(std::find(state.discard.begin(), state.discard.end(),
                   sim::Card::HisuianHeavyBall) == state.discard.end(),
         "Fixture accidentally retained Hisuian Heavy Ball provenance");
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // K1 is the knowledge state. Its legal provenance is not restricted to Hisuian
  // Heavy Ball, while the first player's T1 Supporter restriction banks Steven until
  // T2 and the semantic continuation finishes on T3.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Advanced first-turn and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "K1 route still required Heavy Ball provenance");
  const auto finish = sim::EngineTestAccess::projected_finish(fixture.engine);
  expect(finish && *finish == 3,
         "Going-first T1 bank did not project the relative T3 finish");
}

void going_second_uses_current_supporter_and_projects_t2_finish() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());

  // Going second permits a T1 Supporter, so Wonder Tag may find Steven and the
  // current turn may play it. The #3316 finish is therefore the immediately
  // following T2 rather than the original witness's absolute T3.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "Going-second current-Supporter connector was rejected");
  const auto finish = sim::EngineTestAccess::projected_finish(fixture.engine);
  expect(finish && *finish == 2,
         "Going-second connector did not project a T2 finish");
}

void later_equivalent_turn_is_relative() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true, 4)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state(3));
  const auto finish = sim::EngineTestAccess::projected_finish(fixture.engine);

  // Once current Supporter legality is satisfied, the route depends on a one-turn
  // continuation horizon rather than the historical T1 coordinate.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "Later equivalent connector state was rejected");
  expect(finish && *finish == 4,
         "Later connector did not project current turn plus one");
}

void matchup_flex_uses_same_ready_turn_contract() {
  Fixture fixture{scenario(sim::DciProfile::MatchupFlexJit)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());

  // Both JIT profiles require the Dragon payload to enter discard on the ready turn.
  // Earthen Vessel supplies that event in the shared #3316 continuation.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::quick_route(fixture.engine),
         "MatchupFlexJit was rejected by the connector contract");
}

void live_quick_ball_preserves_package_and_fetches_tapu() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());

  // The confirmed dynamic-DCI payment spends Tate & Liza while preserving the held
  // VSTAR and two Grass, then Quick Ball searches Tapu Lele-GX.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
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

void wonder_tag_selector_reuses_the_same_contract() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, wonder_state());

  // The post-Quick-Ball state differs only by physical transaction provenance.
  // Wonder Tag must therefore select Steven under the same route contract.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(sim::EngineTestAccess::wonder_route(fixture.engine),
         "Post-Quick-Ball route diverged from the shared contract");
  expect(sim::EngineTestAccess::choose_supporter(fixture.engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not select Steven's Resolve");
}

void k0_is_rejected() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, quick_state(), false);
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Connector read fixed-list deck or Prize identities at K0");
}

void no_discard_control_is_rejected() {
  Fixture fixture{scenario(sim::DciProfile::NoDiscardControl)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "NoDiscardControl entered the same-ready-turn JIT connector");
}

void current_item_lock_is_rejected() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Quick Ball connector crossed current Item lock");
}

void rule_box_lock_is_rejected_before_wonder_tag() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::FullRuleBoxAbility)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());

  // Wonder Tag is a Rule Box Pokémon Ability, so a live Rule Box Ability lock blocks
  // this connector before the downstream Trainer-only continuation can exist.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Connector ignored current Rule Box Ability lock");
}

void projected_turn_two_item_lock_is_rejected() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::TurnTwoItem, false, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, quick_state());

  // Going second would finish on T2, where scheduled Item lock makes the required
  // Earthen Vessel illegal.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Turn-two Item-lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Connector ignored projected finish-turn Item lock");
}

void supporter_lock_and_spent_slot_are_rejected() {
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::FullSupporter)};
    sim::EngineTestAccess::set_state(fixture.engine, quick_state());
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector crossed full Supporter lock");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 3)};
    sim::State state = quick_state();
    state.supporter_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector ignored current Supporter contention");
  }
}

void full_bench_rejects_pre_search_stage() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 2)};
  sim::State state = quick_state();
  for (int index = 0; index < 5; ++index) {
    state.bench.push_back(
        sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Quick Ball route invented Bench space for Tapu Lele-GX");
}

void invalid_discard_cost_is_rejected() {
  Fixture fixture{scenario(sim::DciProfile::StrictJit,
                           sim::LockMode::None, false, 2)};
  sim::State state = quick_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::TateLiza), state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_route(fixture.engine),
         "Quick Ball route invented an approved dynamic-DCI cost");
}

void missing_resource_and_horizon_are_rejected() {
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 2)};
    sim::State state = quick_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::EarthenVessel), state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Connector invented Earthen Vessel access");
  }
  {
    Fixture fixture{scenario(sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 2)};
    sim::EngineTestAccess::set_state(fixture.engine, quick_state());
    expect(!sim::EngineTestAccess::quick_route(fixture.engine),
           "Going-first bank exceeded the configured T2 horizon");
  }
}

}  // namespace

int main() {
  try {
    first_turn_bank_accepts_k1_without_heavy_ball_provenance();
    going_second_uses_current_supporter_and_projects_t2_finish();
    later_equivalent_turn_is_relative();
    matchup_flex_uses_same_ready_turn_contract();
    live_quick_ball_preserves_package_and_fetches_tapu();
    wonder_tag_selector_reuses_the_same_contract();
    k0_is_rejected();
    no_discard_control_is_rejected();
    current_item_lock_is_rejected();
    rule_box_lock_is_rejected_before_wonder_tag();
    projected_turn_two_item_lock_is_rejected();
    supporter_lock_and_spent_slot_are_rejected();
    full_bench_rejects_pre_search_stage();
    invalid_discard_cost_is_rejected();
    missing_resource_and_horizon_are_rejected();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
