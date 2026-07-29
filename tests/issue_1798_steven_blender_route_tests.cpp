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
  static bool route_available(const Engine& engine) {
    return engine.issue_1798_steven_blender_route_available();
  }
  static bool start_route(Engine& engine) {
    return engine.start_issue_1798_steven_blender_route();
  }
  static bool finish_route(Engine& engine) {
    return engine.complete_issue_1798_steven_blender_route();
  }
  static State& state(Engine& engine) { return engine.state_; }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(
    const sim::LockMode locks = sim::LockMode::None,
    const bool going_first = true,
    const int max_turn = 4) {
  return sim::Scenario{"issue-1798-steven-blender", sim::DciProfile::StrictJit,
                       locks, going_first, max_turn};
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::Dragapult,
      sim::Card::Powerglass,
      sim::Card::BrilliantBlender,
      sim::Card::RoseannesBackup,
      sim::Card::Dipplin,
      sim::Card::Fire,
      sim::Card::StevensResolve,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::RegidragoVstar,
      sim::Card::MegaDragonite,
      sim::Card::QuickBall,
      sim::Card::ErikasInvitation,
  };
  state.prizes = {
      sim::Card::Grass,
      sim::Card::GoodraVstar,
      sim::Card::RegidragoVstar,
      sim::Card::MysteriousTreasure,
      sim::Card::ProfessorBurnet,
      sim::Card::DialgaGX,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          sim::DeckRecipe selected_recipe = sim::baseline_recipe(),
          const std::uint64_t seed = 1798)
      : scenario_value(std::move(selected_scenario)),
        recipe(std::move(selected_recipe)),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void route_attaches_fire_then_finishes_with_deck_payload() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // K1 proves the VSTAR, final Grass, and a permitted Dragon remain in the deck.
  // The held Fire is a required total attachment even though Grass is the missing
  // type. Brilliant Blender must use the deck payload and preserve held Dragapult ex:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1798
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The complete Steven and Blender route was not recognized");
  expect(sim::EngineTestAccess::start_route(fixture.engine),
         "The T3 route did not start");

  sim::State& after_steven = sim::EngineTestAccess::state(fixture.engine);
  expect(after_steven.active->grass == 1 && after_steven.active->fire == 2,
         "The T3 Fire attachment did not reach GFF");
  expect(after_steven.manual_energy_used,
         "The T3 manual attachment was not consumed");
  expect(after_steven.turn_ended, "Steven did not end T3");
  expect(contains(after_steven.discard, sim::Card::StevensResolve),
         "Steven did not enter discard");
  expect(contains(after_steven.hand, sim::Card::RegidragoVstar),
         "Steven did not search Regidrago VSTAR");
  expect(contains(after_steven.hand, sim::Card::Grass),
         "Steven did not search the final Grass");
  expect(contains(after_steven.hand, sim::Card::Dragapult),
         "The held Dragon was consumed before Blender");

  after_steven.turn = 4;
  after_steven.turn_ended = false;
  after_steven.supporter_used = false;
  after_steven.manual_energy_used = false;
  after_steven.discarded_this_turn.clear();

  expect(sim::EngineTestAccess::finish_route(fixture.engine),
         "The committed T4 route did not finish");
  const sim::State& ready = sim::EngineTestAccess::state(fixture.engine);
  expect(ready.active->card == sim::Card::RegidragoVstar,
         "The Active did not evolve into Regidrago VSTAR");
  expect(ready.active->grass == 2 && ready.active->fire == 2,
         "The final Grass attachment did not reach GGFF");
  expect(contains(ready.discard, sim::Card::BrilliantBlender),
         "Brilliant Blender did not enter discard");
  expect(contains(ready.discard, sim::Card::MegaDragonite),
         "Brilliant Blender did not discard the K1-proven deck payload");
  expect(contains(ready.hand, sim::Card::Dragapult),
         "Brilliant Blender incorrectly consumed the held Dragon");
}

void held_only_payload_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::MegaDragonite),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // Brilliant Blender selects cards from the deck. A held Dragon cannot satisfy its
  // strict-JIT payload channel: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regression refinement: https://github.com/FlareZ123/pokemon-sims/issues/1798
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route treated a held Dragon as a Blender deck payload");
}

void k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state(), false);
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route read deck or Prize identities at K0");
}

void missing_fire_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented the T3 Fire attachment");
}

void missing_steven_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.erase(
      std::find(state.hand.begin(), state.hand.end(), sim::Card::StevensResolve));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented Steven's Resolve");
}

void missing_blender_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.erase(
      std::find(state.hand.begin(), state.hand.end(), sim::Card::BrilliantBlender));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route invented Brilliant Blender");
}

void missing_vstar_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::RegidragoVstar),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Steven invented an absent Regidrago VSTAR");
}

void missing_grass_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Steven invented an absent Grass Energy");
}

void spent_attachment_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route reused the T3 manual attachment");
}

void current_turn_active_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.active->entered_turn = 3;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route ignored evolution timing");
}

void lock_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_state(fixture.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route projected Brilliant Blender through Item lock");
}

void expired_horizon_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::None, true, 3)};
  sim::EngineTestAccess::set_state(fixture.engine, base_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route exceeded the configured setup horizon");
}

void held_direct_vstar_connector_stays_ahead() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(sim::Card::RegidragoVstar);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // A direct current-turn evolution remains ahead of a deferred Steven package:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1798
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The deferred route displaced a held direct VSTAR connector");
}

void exact_seed_reaches_turn_four() {
  const auto selected_scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-first scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{1801};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // The source-bound seed must use only public K1 information and avoid the random
  // second Celestial Roar attack:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1798
  expect(outcome.first_ready_turn == 4,
         "Seed 1801 did not reach the deterministic T4 route");
  expect(trace_contains("deterministic T4 Steven and Blender route"),
         "Seed 1801 did not attach Fire before Steven");
  expect(trace_contains("committed T4 Brilliant Blender finish"),
         "Seed 1801 did not bank the complete Steven package");
  expect(trace_contains("T4 | READY"),
         "Seed 1801 did not become ready on T4");
  expect(!trace_contains("T3 | ATTACK"),
         "Seed 1801 still used the random T3 Celestial Roar attack");
}

}  // namespace

int main() {
  try {
    route_attaches_fire_then_finishes_with_deck_payload();
    held_only_payload_rejects_route();
    k0_rejects_route();
    missing_fire_rejects_route();
    missing_steven_rejects_route();
    missing_blender_rejects_route();
    missing_vstar_rejects_route();
    missing_grass_rejects_route();
    spent_attachment_rejects_route();
    current_turn_active_rejects_route();
    lock_rejects_route();
    expired_horizon_rejects_route();
    held_direct_vstar_connector_stays_ahead();
    exact_seed_reaches_turn_four();
    std::cout << "issue 1798 Steven Blender route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "issue 1798 Steven Blender route tests failed: "
              << error.what() << '\n';
    return 1;
  }
}
