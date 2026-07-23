#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }

  static std::vector<Card> lusamine_targets(const Engine& engine) {
    return engine.lusamine_recovery_targets();
  }

  static bool play_lusamine(Engine& engine) { return engine.play_lusamine(); }
};
}  // namespace sim

namespace {

sim::Scenario rule_box_scenario(const int max_turn = 5) {
  return sim::Scenario{
      "issue-1392/exact",
      sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility,
      false,
      max_turn,
  };
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Lusamine,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Fire,
  };
  state.discard = {
      sim::Card::EarthenVessel,
      sim::Card::EarthenVessel,
      sim::Card::ChaoticSwell,
      sim::Card::Dragapult,
      sim::Card::Guzma,
  };
  state.deck = {
      sim::Card::QuickBall,
      sim::Card::MysteriousTreasure,
      sim::Card::TapuLeleGX,
      sim::Card::ProfessorBurnet,
      sim::Card::MegaDragonite,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Fire,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(sim::Scenario value = rule_box_scenario())
      : scenario(std::move(value)),
        recipe(sim::baseline_recipe()),
        rng(1392),
        engine(scenario, recipe, rng) {}
};

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_swell_is_advancing_first_target() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());
  const auto targets = sim::EngineTestAccess::lusamine_targets(fixture.engine);

  // Lusamine must recover exactly two Supporter/Stadium cards. Under active Path,
  // Chaotic Swell is the advancing first target because playing it restores
  // Regidrago VSTAR's Legacy Star Ability:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1392
  require(targets.size() == 2U && targets[0] == sim::Card::ChaoticSwell &&
              targets[1] == sim::Card::Guzma,
          "Lusamine did not select Chaotic Swell plus the mandatory second target.");
}

void test_no_lock_and_spent_resources_block_route() {
  {
    sim::Scenario scenario = rule_box_scenario();
    scenario.locks = sim::LockMode::None;
    Fixture fixture(scenario);
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());
    require(sim::EngineTestAccess::lusamine_targets(fixture.engine).empty(),
            "Chaotic Swell was promoted without a Rule Box Ability lock.");
  }

  for (const int mode : {0, 1}) {
    Fixture fixture;
    sim::State state = exact_state();
    if (mode == 0) state.stadium_used = true;
    if (mode == 1) state.vstar_power_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

    // A used Stadium play or used VSTAR Power cannot support the unlock line:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1392
    require(sim::EngineTestAccess::lusamine_targets(fixture.engine).empty(),
            "Chaotic Swell was promoted after a required resource was spent.");
  }
}

void test_horizon_and_exact_two_target_requirement() {
  {
    Fixture fixture(rule_box_scenario(2));
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());
    require(sim::EngineTestAccess::lusamine_targets(fixture.engine).empty(),
            "Chaotic Swell was promoted beyond the configured horizon.");
  }

  {
    Fixture fixture;
    sim::State state = exact_state();
    state.discard = {sim::Card::ChaoticSwell};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    const auto targets = sim::EngineTestAccess::lusamine_targets(fixture.engine);

    // Lusamine says 2, rather than up to 2, so a lone Swell is insufficient:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/issues/1392
    require(targets.size() == 1U && !sim::EngineTestAccess::play_lusamine(fixture.engine),
            "Lusamine resolved without its mandatory second target.");
  }
}

void test_seed_20_reaches_t4() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-rulebox-ability-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  require(scenario.has_value() && deck != nullptr,
          "The issue-1392 source-bound fixture is unavailable.");

  std::mt19937_64 rng(20);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  require(outcome.first_ready_turn == 4,
          "Seed 20 did not improve from failure to T4 readiness.");

  const auto has_line = [&trace](const std::string& fragment) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&fragment](const std::string& line) {
                         return line.find(fragment) != std::string::npos;
                       });
  };

  // The exact trace must recover and play Swell, then use Legacy Star:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1392
  require(has_line("Recovered from discard: Chaotic Swell, Guzma") &&
              has_line("Played Chaotic Swell, replacing the modeled Path") &&
              has_line("LEGACY STAR") && has_line("T4 | READY"),
          "Seed 20 lost the confirmed Chaotic Swell unlock trace.");
}

}  // namespace

int main() {
  test_swell_is_advancing_first_target();
  test_no_lock_and_spent_resources_block_route();
  test_horizon_and_exact_two_target_requirement();
  test_seed_20_reaches_t4();
}
