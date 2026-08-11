#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void mark_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool hold_blender_for_burnet(const Engine& engine) {
    return engine.issue_1646_hold_blender_for_burnet_finish_visible();
  }
  static bool play_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

struct Fixture {
  Fixture(const sim::DciProfile dci, const sim::LockMode lock,
          const bool going_first)
      : scenario{"issue-3010/exact", dci, lock, going_first, 5},
        recipe{sim::baseline_recipe()},
        rng{3010},
        trace{true, {}},
        engine{scenario, recipe, rng, &trace} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;
};

sim::State burnet_finish_state(const int turn = 2) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None, 0};
  state.hand = {
      sim::Card::Grass,
      sim::Card::ProfessorBurnet,
      sim::Card::BrilliantBlender,
  };
  state.deck = {
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
      sim::Card::Fire,
  };
  state.prizes = {
      sim::Card::ForestSealStone,
      sim::Card::FieldBlower,
      sim::Card::Oricorio,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::QuickBall,
  };
  return state;
}

void install(Fixture& fixture, sim::State state) {
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::mark_deck_seen(fixture.engine);
}

void test_matchup_flex_rulebox_later_turn_holds_blender() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit,
                  sim::LockMode::FullRuleBoxAbility, false};
  install(fixture, burnet_finish_state(4));

  // MatchupFlexJit has the same ready-turn payload requirement as StrictJit.
  // Professor Burnet deterministically supplies that payload, the held Grass plus
  // unused manual attachment finishes GGF, and Rule Box Ability lock does not stop
  // either Trainer card. Spending the singleton ACE SPEC is therefore dominated:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, Supporter, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Same-turn JIT and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/3010
  expect(sim::EngineTestAccess::hold_blender_for_burnet(fixture.engine),
         "MatchupFlexJit did not preserve Blender for the visible Burnet finish.");
  expect(!sim::EngineTestAccess::play_blender(fixture.engine),
         "Brilliant Blender was spent despite the equal-turn Burnet route.");
  expect(count(sim::EngineTestAccess::state(fixture.engine).hand,
               sim::Card::BrilliantBlender) == 1,
         "The ACE SPEC left hand while the Burnet route dominated it.");
  expect(trace_contains(fixture.trace, "HOLD ITEM") &&
             trace_contains(fixture.trace, "P-COMPRESS-01"),
         "The dominance hold was not recorded in the trace.");
}

void test_strict_jit_does_not_require_historical_breadcrumbs() {
  Fixture fixture{sim::DciProfile::StrictJit, sim::LockMode::None, false};
  install(fixture, burnet_finish_state(2));

  // The live decision depends on the current board/hand/deck route. Earthen Vessel
  // and Quick Ball discard breadcrumbs from the original seed are not legality or
  // resource requirements once Active VSTAR, final Grass, Burnet, and payload are
  // already observable:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Original witness: https://github.com/FlareZ123/pokemon-sims/issues/1646
  // Systemic correction: https://github.com/FlareZ123/pokemon-sims/issues/3010
  expect(sim::EngineTestAccess::state(fixture.engine).discard.empty(),
         "The exact-state control unexpectedly contains discard breadcrumbs.");
  expect(sim::EngineTestAccess::hold_blender_for_burnet(fixture.engine),
         "The hold still depended on historical turn/discard coordinates.");
}

void test_post_attachment_state_still_holds_blender() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit, sim::LockMode::None, true};
  sim::State state = burnet_finish_state(3);
  state.manual_energy_used = true;
  state.active->grass = 2;
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass));
  install(fixture, std::move(state));

  // Once GGF is already complete, Burnet remains the lower-resource current-turn
  // payload route and Blender must stay protected:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/3010
  expect(sim::EngineTestAccess::hold_blender_for_burnet(fixture.engine),
         "The post-attachment Burnet finish did not preserve Blender.");
}

void test_live_route_requirements_remain_enforced() {
  const auto expect_blocked = [](sim::DciProfile dci, sim::LockMode lock,
                                 sim::State state, const char* message) {
    Fixture fixture{dci, lock, false};
    install(fixture, std::move(state));
    expect(!sim::EngineTestAccess::hold_blender_for_burnet(fixture.engine), message);
  };

  // Negative controls retain the actual legality/resource boundaries. A Supporter
  // already used, missing Burnet/payload, unavailable final attachment, Item lock,
  // or a profile without ready-turn payload timing means the Burnet-dominance hold
  // is not established:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // JIT/Supporter/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/3010
  sim::State supporter_spent = burnet_finish_state();
  supporter_spent.supporter_used = true;
  expect_blocked(sim::DciProfile::StrictJit, sim::LockMode::None,
                 std::move(supporter_spent),
                 "The hold ignored an already-used Supporter slot.");

  sim::State no_burnet = burnet_finish_state();
  no_burnet.hand.erase(std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                                 sim::Card::ProfessorBurnet));
  expect_blocked(sim::DciProfile::StrictJit, sim::LockMode::None,
                 std::move(no_burnet),
                 "The hold activated without Professor Burnet.");

  sim::State no_payload = burnet_finish_state();
  no_payload.deck.erase(std::find(no_payload.deck.begin(), no_payload.deck.end(),
                                  sim::Card::Dragapult));
  expect_blocked(sim::DciProfile::StrictJit, sim::LockMode::None,
                 std::move(no_payload),
                 "The hold activated without a Burnet-searchable payload.");

  sim::State no_energy_finish = burnet_finish_state();
  no_energy_finish.hand.erase(
      std::find(no_energy_finish.hand.begin(), no_energy_finish.hand.end(),
                sim::Card::Grass));
  expect_blocked(sim::DciProfile::StrictJit, sim::LockMode::None,
                 std::move(no_energy_finish),
                 "The hold activated without a finishing Energy route.");

  expect_blocked(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                 burnet_finish_state(),
                 "The hold reported Blender dominance while Items were locked.");

  expect_blocked(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                 burnet_finish_state(),
                 "The hold ignored the profile's lack of ready-turn JIT timing.");
}

}  // namespace

int main() {
  try {
    test_matchup_flex_rulebox_later_turn_holds_blender();
    test_strict_jit_does_not_require_historical_breadcrumbs();
    test_post_attachment_state_still_holds_blender();
    test_live_route_requirements_remain_enforced();
    std::cout << "Issue 3010 Burnet-Blender dominance tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
