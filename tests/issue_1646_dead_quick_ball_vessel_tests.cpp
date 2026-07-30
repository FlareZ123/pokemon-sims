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
  static std::optional<Card> issue_1866_cost(const Engine& engine) {
    return engine.issue_1866_vessel_burnet_cost();
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
  explicit Fixture(const sim::LockMode lock = sim::LockMode::None)
      : scenario{"issue-1866/exact", sim::DciProfile::StrictJit, lock, true, 5},
        recipe{sim::baseline_recipe()},
        rng{1866},
        trace{true, {}},
        engine{scenario, recipe, rng, &trace} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;
};

sim::State grass_finish_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, 2, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::ProfessorBurnet,
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
  return state;
}

sim::State fire_finish_state() {
  sim::State state = grass_finish_state();
  state.active->grass = 2;
  state.active->fire = 0;
  state.hand = {
      sim::Card::ProfessorBurnet,
      sim::Card::EarthenVessel,
      sim::Card::Guzma,
  };
  return state;
}

void install_state(Fixture& fixture, sim::State state, const bool k1 = true) {
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (k1) sim::EngineTestAccess::mark_deck_seen(fixture.engine);
}

void test_blender_free_grass_finish_spends_quick_ball() {
  Fixture fixture;
  install_state(fixture, grass_finish_state());

  // Quick Ball's Basic-search role is complete because Active Regidrago VSTAR
  // already exists. Earthen Vessel searches the final Grass, Professor Burnet
  // supplies the current-turn Dragon payload, and the manual attachment finishes
  // GGF. Brilliant Blender is absent and the route remains deterministic:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
  expect(sim::EngineTestAccess::issue_1646_route(fixture.engine),
         "The Blender-free Grass finish was not recognized.");
  expect(sim::EngineTestAccess::issue_1866_cost(fixture.engine) ==
             sim::Card::QuickBall,
         "The route did not select Quick Ball as its replaced connector.");
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "Earthen Vessel did not resolve the Blender-free Grass finish.");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(count(after.hand, sim::Card::Grass) == 1,
         "Earthen Vessel did not search the final Grass Energy.");
  expect(count(after.discard, sim::Card::QuickBall) == 1 &&
             count(after.discard, sim::Card::EarthenVessel) == 1,
         "Earthen Vessel did not pay with Quick Ball.");
  expect(count(after.discard, sim::Card::ProfessorBurnet) == 1 &&
             count(after.discard, sim::Card::Dragapult) == 1,
         "The route did not resolve Professor Burnet for a Dragon payload.");
  expect(trace_contains(
             fixture.trace,
             "Discarded Quick Ball as the route-replaced cost and searched Grass Energy") &&
             trace_contains(fixture.trace,
                            "PLAY SUPPORTER | rules: R-BURNET-01"),
         "The Grass-finish DCI and Burnet choices were not recorded.");
}

void test_fire_finish_spends_guzma() {
  Fixture fixture;
  install_state(fixture, fire_finish_state());

  // Guzma is opponent-dependent in this goldfish setup model. The visible
  // Vessel-Burnet line reaches readiness immediately, which raises Guzma's
  // route-specific DCI while Basic Energy, payloads, recovery, and ACE SPEC
  // resources stay protected:
  // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
  expect(sim::EngineTestAccess::issue_1646_route(fixture.engine),
         "The final-Fire orientation was not recognized.");
  expect(sim::EngineTestAccess::issue_1866_cost(fixture.engine) ==
             sim::Card::Guzma,
         "The final-Fire route did not select Guzma.");
  expect(sim::EngineTestAccess::play_vessel(fixture.engine),
         "Earthen Vessel did not resolve the final-Fire orientation.");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(count(after.hand, sim::Card::Fire) == 1 &&
             count(after.discard, sim::Card::Guzma) == 1,
         "The final-Fire route searched Fire or paid its cost incorrectly.");
}

void test_later_turn_spends_erikas_invitation() {
  Fixture fixture;
  sim::State state = grass_finish_state();
  state.turn = 4;
  state.hand = {
      sim::Card::ProfessorBurnet,
      sim::Card::EarthenVessel,
      sim::Card::ErikasInvitation,
  };
  install_state(fixture, std::move(state));

  // The same exact graph remains legal on T4. Erika's Invitation has no
  // setup effect in the repository's opponent-free model and can pay Vessel
  // once the deterministic Burnet finish is visible:
  // Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Repository model boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
  // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
  expect(sim::EngineTestAccess::issue_1646_route(fixture.engine),
         "The later-turn Grass finish was not recognized.");
  expect(sim::EngineTestAccess::issue_1866_cost(fixture.engine) ==
             sim::Card::ErikasInvitation,
         "The later-turn route did not select Erika's Invitation.");
}

void test_live_axes_and_hidden_information_block_route() {
  const auto expect_blocked = [](sim::State state, const bool k1,
                                 const char* message) {
    Fixture fixture;
    install_state(fixture, std::move(state), k1);
    expect(!sim::EngineTestAccess::issue_1646_route(fixture.engine), message);
  };

  sim::State no_burnet = grass_finish_state();
  no_burnet.hand.erase(
      std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                sim::Card::ProfessorBurnet));
  expect_blocked(std::move(no_burnet), true,
                 "The route activated without Professor Burnet.");

  sim::State no_safe_cost = grass_finish_state();
  no_safe_cost.hand = {
      sim::Card::ProfessorBurnet,
      sim::Card::EarthenVessel,
      sim::Card::BrilliantBlender,
      sim::Card::Klara,
      sim::Card::Grass,
  };
  expect_blocked(std::move(no_safe_cost), true,
                 "The route spent protected resources without a safe cost.");

  sim::State manual_spent = grass_finish_state();
  manual_spent.manual_energy_used = true;
  expect_blocked(std::move(manual_spent), true,
                 "The route ignored the spent manual attachment.");

  sim::State no_payload = grass_finish_state();
  no_payload.deck.erase(
      std::find(no_payload.deck.begin(), no_payload.deck.end(),
                sim::Card::Dragapult));
  expect_blocked(std::move(no_payload), true,
                 "The route activated without a Burnet-searchable payload.");

  sim::State no_energy = grass_finish_state();
  no_energy.deck.erase(
      std::find(no_energy.deck.begin(), no_energy.deck.end(),
                sim::Card::Grass));
  expect_blocked(std::move(no_energy), true,
                 "The route activated without the missing Basic Energy.");

  sim::State energy_complete = grass_finish_state();
  energy_complete.active->grass = 2;
  expect_blocked(std::move(energy_complete), true,
                 "The route activated after GGF was complete.");

  expect_blocked(grass_finish_state(), false,
                 "The route inspected deck targets from K0.");
}

void test_item_lock_blocks_resolution() {
  Fixture fixture{sim::LockMode::FullItem};
  install_state(fixture, grass_finish_state());

  // Full Item lock prevents Earthen Vessel from being played:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Repository lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
  expect(!sim::EngineTestAccess::issue_1646_route(fixture.engine),
         "The route ignored the permanent Item lock.");
  expect(!sim::EngineTestAccess::play_vessel(fixture.engine),
         "Earthen Vessel resolved through permanent Item lock.");
}

void expect_seed_ready(const std::string_view scenario_label,
                       const std::uint64_t seed,
                       const int expected_turn,
                       const sim::Card expected_cost,
                       const sim::Card expected_energy) {
  const auto scenario = sim::scenario_by_label(std::string(scenario_label));
  expect(scenario.has_value(), "The requested source-bound scenario is unavailable.");

  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine{*scenario, recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  const std::string cost_text =
      std::string(sim::name(expected_cost)) + " (Earthen Vessel cost)";
  const std::string energy_text =
      "searched " + std::string(sim::name(expected_energy)) +
      " for the held Professor Burnet finish";

  // These registered-shell traces exercise both final-Energy orientations and
  // the later-turn boundary on the exact issue witnesses:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Refined confirmed regression and seeds: https://github.com/FlareZ123/pokemon-sims/issues/1866
  if (outcome.first_ready_turn != expected_turn || outcome.setup_failed ||
      !trace_contains(trace, cost_text) ||
      !trace_contains(trace, energy_text) ||
      !trace_contains(trace, "PLAY SUPPORTER | rules: R-BURNET-01") ||
      !trace_contains(trace, "READY")) {
    for (const std::string& line : trace.lines) std::cerr << line << '\n';
    throw std::runtime_error(
        "A source-bound issue-1866 trace did not complete on its proven turn.");
  }
}

void test_registered_shell_issue_1866_witnesses() {
  expect_seed_ready("strict-jit/go-first", 40, 3,
                    sim::Card::QuickBall, sim::Card::Grass);
  expect_seed_ready("strict-jit/go-first", 138, 3,
                    sim::Card::Guzma, sim::Card::Fire);
  expect_seed_ready("strict-jit/go-second", 398, 4,
                    sim::Card::ErikasInvitation, sim::Card::Grass);
}

void test_original_issue_1646_seed_remains_ready() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario.has_value() && deck != nullptr,
         "The original issue-1646 seed fixture is unavailable.");

  std::mt19937_64 rng{245};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  // The original modeling witness keeps Brilliant Blender and still pays
  // Vessel with Quick Ball before Professor Burnet supplies the payload:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Original confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1646
  if (outcome.first_ready_turn != 3 || outcome.setup_failed ||
      !trace_contains(trace, "Quick Ball (Earthen Vessel cost)") ||
      !trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-BURNET-01") ||
      !trace_contains(trace, "T3 | READY") ||
      trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01")) {
    for (const std::string& line : trace.lines) std::cerr << line << '\n';
    throw std::runtime_error(
        "The original issue-1646 route regressed.");
  }
}

}  // namespace

int main() {
  try {
    test_blender_free_grass_finish_spends_quick_ball();
    test_fire_finish_spends_guzma();
    test_later_turn_spends_erikas_invitation();
    test_live_axes_and_hidden_information_block_route();
    test_item_lock_blocks_resolution();
    test_registered_shell_issue_1866_witnesses();
    test_original_issue_1646_seed_remains_ready();
    std::cout << "Issue 1646 and 1866 Vessel-Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
