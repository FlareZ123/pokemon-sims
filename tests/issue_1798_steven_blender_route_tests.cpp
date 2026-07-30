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
    const int max_turn = 4) {
  return sim::Scenario{"issue-1798-steven-blender", sim::DciProfile::StrictJit,
                       locks, true, max_turn};
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

  explicit Fixture(sim::Scenario selected_scenario = scenario())
      : scenario_value(std::move(selected_scenario)),
        recipe(sim::baseline_recipe()),
        rng(1798),
        engine(scenario_value, recipe, rng) {}
};

void route_attaches_fire_then_finishes_with_deck_payload() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  // K1 proves the VSTAR, final Grass, and a permitted Dragon remain in the deck.
  // Brilliant Blender must use the deck payload and preserve held Dragapult ex:
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
  expect(after_steven.turn_ended, "Steven did not end T3");
  expect(contains(after_steven.hand, sim::Card::RegidragoVstar) &&
             contains(after_steven.hand, sim::Card::Grass),
         "Steven did not bank the VSTAR and final Grass");
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
  expect(ready.active->card == sim::Card::RegidragoVstar &&
             ready.active->grass == 2 && ready.active->fire == 2,
         "The Active did not evolve and reach GGFF");
  expect(contains(ready.discard, sim::Card::BrilliantBlender) &&
             contains(ready.discard, sim::Card::MegaDragonite),
         "Blender did not discard the K1-proven deck payload");
  expect(contains(ready.hand, sim::Card::Dragapult),
         "Blender incorrectly consumed the held Dragon");
}

void held_only_payload_rejects_route() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::MegaDragonite),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // Brilliant Blender selects from the deck, so a held Dragon cannot satisfy it:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1798
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route treated a held Dragon as a Blender deck payload");
}

void knowledge_lock_timing_and_precedence_controls() {
  {
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, base_state(), false);
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route read deck or Prize identities at K0");
  }
  {
    Fixture fixture{scenario(sim::LockMode::FullItem)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route projected Blender through Item lock");
  }
  {
    Fixture fixture{scenario(sim::LockMode::None, 3)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route exceeded the configured horizon");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.active->entered_turn = 3;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The route ignored evolution timing");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.hand.push_back(sim::Card::RegidragoVstar);
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    // A direct current-turn evolution remains ahead of a deferred package:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "The deferred route displaced a direct VSTAR connector");
  }
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

  // The source-bound seed uses public K1 information and avoids a random T3 attack:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1798
  expect(outcome.first_ready_turn == 4,
         "Seed 1801 did not reach the deterministic T4 route");
  expect(trace_contains("deterministic T4 Steven and Blender route") &&
             trace_contains("committed T4 Brilliant Blender finish") &&
             trace_contains("T4 | READY"),
         "Seed 1801 did not execute the complete public route");
  expect(!trace_contains("T3 | ATTACK"),
         "Seed 1801 still used the random T3 Celestial Roar attack");
}

}  // namespace

int main() {
  try {
    route_attaches_fire_then_finishes_with_deck_payload();
    held_only_payload_rejects_route();
    knowledge_lock_timing_and_precedence_controls();
    exact_seed_reaches_turn_four();
    std::cout << "issue 1798 Steven Blender route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "issue 1798 Steven Blender route tests failed: "
              << error.what() << '\n';
    return 1;
  }
}
