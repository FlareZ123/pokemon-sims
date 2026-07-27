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
  static bool issue_1646_route(const Engine& engine) {
    return engine.issue_1646_vessel_burnet_finish_visible();
  }
  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
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
  sim::Scenario scenario{"issue-1646/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1646};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{scenario, recipe, rng, &trace};
};

sim::State complete_route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, 2, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RoseannesBackup,
      sim::Card::ProfessorBurnet,
      sim::Card::BrilliantBlender,
      sim::Card::LatiasEx,
      sim::Card::EarthenVessel,
      sim::Card::QuickBall,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
  };
  state.prizes = {
      sim::Card::ForestSealStone,
      sim::Card::FieldBlower,
      sim::Card::Oricorio,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::QuickBall,
  };
  state.discard = {
      sim::Card::MysteriousTreasure,
      sim::Card::MysteriousTreasure,
      sim::Card::Crispin,
  };
  return state;
}

void install_complete_state(Fixture& fixture, sim::State state = complete_route_state()) {
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::mark_deck_seen(fixture.engine);
}

void test_exact_state_spends_dominated_quick_ball() {
  Fixture fixture;
  install_complete_state(fixture);

  // The Active Regidrago VSTAR has only the final Grass and current-turn payload
  // axes unresolved. Earthen Vessel can pay its printed discard cost with the
  // route-dominated Quick Ball, while Professor Burnet remains available for the
  // Supporter-limited strict-JIT payload and Brilliant Blender stays protected:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1646
  expect(sim::EngineTestAccess::issue_1646_route(fixture.engine),
         "The exact K1 Vessel-Burnet route was not recognized.");
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "Earthen Vessel did not resolve the exact issue-1646 route.");
  expect(!sim::EngineTestAccess::play_blender(fixture.engine),
         "Brilliant Blender duplicated the held Burnet payload route.");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(count(after.hand, sim::Card::Grass) == 1,
         "Earthen Vessel did not search the final Grass Energy.");
  expect(count(after.discard, sim::Card::QuickBall) == 1 &&
             count(after.discard, sim::Card::EarthenVessel) == 1,
         "Earthen Vessel did not pay with the dominated Quick Ball.");
  expect(count(after.hand, sim::Card::ProfessorBurnet) == 1 &&
             count(after.hand, sim::Card::BrilliantBlender) == 1,
         "The route failed to preserve Burnet and the ACE SPEC.");
  expect(trace_contains(fixture.trace,
                        "Discarded the dominated Quick Ball and searched Grass Energy") &&
             trace_contains(fixture.trace,
                            "Held Brilliant Blender because Professor Burnet completes"),
         "The exact DCI decisions were not recorded in the trace.");
}

void test_incomplete_axes_do_not_enable_exact_route() {
  const auto expect_blocked = [](sim::State state, const char* message) {
    Fixture fixture;
    install_complete_state(fixture, std::move(state));
    expect(!sim::EngineTestAccess::issue_1646_route(fixture.engine), message);
  };

  sim::State no_burnet = complete_route_state();
  no_burnet.hand.erase(std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                                 sim::Card::ProfessorBurnet));
  expect_blocked(std::move(no_burnet),
                 "Quick Ball became discardable without the paid payload outlet.");

  sim::State no_blender_fallback = complete_route_state();
  no_blender_fallback.hand.erase(
      std::find(no_blender_fallback.hand.begin(), no_blender_fallback.hand.end(),
                sim::Card::BrilliantBlender));
  expect_blocked(std::move(no_blender_fallback),
                 "The narrow seed-245 guard expanded without its observed ACE SPEC fallback.");

  sim::State manual_spent = complete_route_state();
  manual_spent.manual_energy_used = true;
  expect_blocked(std::move(manual_spent),
                 "The route ignored the spent manual attachment.");

  sim::State no_payload = complete_route_state();
  no_payload.deck.erase(std::find(no_payload.deck.begin(), no_payload.deck.end(),
                                  sim::Card::Dragapult));
  expect_blocked(std::move(no_payload),
                 "The route activated without a Burnet-searchable payload.");

  sim::State energy_complete = complete_route_state();
  energy_complete.active->grass = 2;
  expect_blocked(std::move(energy_complete),
                 "The exact one-Grass route activated after GGF was complete.");
}

void test_seed_245_reaches_strict_jit_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario.has_value() && deck != nullptr,
         "The source-bound issue-1646 seed fixture is unavailable.");

  std::mt19937_64 rng{245};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  // Seed 245 must use the held Vessel and Burnet, pay Vessel with Quick Ball,
  // attach the searched Grass, and preserve Brilliant Blender while reaching T3:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core Supporter and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1646
  if (outcome.first_ready_turn != 3 || outcome.setup_failed ||
      !trace_contains(trace, "Quick Ball (Earthen Vessel cost)") ||
      !trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-BURNET-01") ||
      !trace_contains(trace, "T3 | READY") ||
      trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01")) {
    for (const std::string& line : trace.lines) std::cerr << line << '\n';
    throw std::runtime_error(
        "Seed 245 did not complete the resource-preserving Vessel-Burnet T3 route.");
  }
}

}  // namespace

int main() {
  try {
    test_exact_state_spends_dominated_quick_ball();
    test_incomplete_axes_do_not_enable_exact_route();
    test_seed_245_reaches_strict_jit_t3();
    std::cout << "Issue 1646 dead Quick Ball Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
