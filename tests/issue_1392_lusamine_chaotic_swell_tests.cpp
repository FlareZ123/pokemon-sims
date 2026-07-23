#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
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

  static bool play_lusamine(Engine& engine) {
    return engine.play_lusamine();
  }

  static bool play_chaotic_swell(Engine& engine) {
    return engine.play_chaotic_swell();
  }

  static bool evolve_best_regi(Engine& engine) {
    return engine.evolve_best_regi();
  }

  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
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
  state.active =
      sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
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

  explicit Fixture(sim::Scenario scenario_value = rule_box_scenario())
      : scenario(std::move(scenario_value)),
        recipe(sim::baseline_recipe()),
        rng(1392),
        engine(scenario, recipe, rng) {}
};

void test_swell_is_the_advancing_first_target() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());

  // Lusamine must recognize discarded Chaotic Swell as the advancing first
  // target while Path suppresses Legacy Star, then use Guzma as its mandatory
  // second legal target:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1392
  const std::vector<sim::Card> targets =
      sim::EngineTestAccess::lusamine_targets(fixture.engine);
  if (targets.size() != 2U ||
      targets[0] != sim::Card::ChaoticSwell ||
      targets[1] != sim::Card::Guzma) {
    throw std::runtime_error(
        "Lusamine did not select Chaotic Swell plus the mandatory second target.");
  }

  if (!sim::EngineTestAccess::play_lusamine(fixture.engine) ||
      !sim::EngineTestAccess::play_chaotic_swell(fixture.engine) ||
      !sim::EngineTestAccess::evolve_best_regi(fixture.engine) ||
      !sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error(
        "The confirmed Chaotic Swell unlock route did not resolve.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.path_lock_removed || !after.vstar_power_used ||
      !after.active || after.active->card != sim::Card::RegidragoVstar) {
    throw std::runtime_error(
        "The Chaotic Swell route failed to restore and use Legacy Star.");
  }
}

void test_no_lock_keeps_swell_fallback_only() {
  sim::Scenario scenario = rule_box_scenario();
  scenario.locks = sim::LockMode::None;
  Fixture fixture(scenario);
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());

  // Chaotic Swell has no advancing unlock value when Path-style suppression is
  // absent:
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/issues/1392
  if (!sim::EngineTestAccess::lusamine_targets(fixture.engine).empty()) {
    throw std::runtime_error(
        "Lusamine promoted Chaotic Swell without a Rule Box Ability lock.");
  }
}

void test_used_resources_block_the_unlock_projection() {
  for (const int mode : {0, 1}) {
    Fixture fixture;
    sim::State state = exact_state();
    if (mode == 0) {
      state.stadium_used = true;
    } else {
      state.vstar_power_used = true;
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

    // A player may play one Stadium per turn and use one VSTAR Power per game.
    // A spent resource cannot support the proposed unlock route:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1392
    if (!sim::EngineTestAccess::lusamine_targets(fixture.engine).empty()) {
      throw std::runtime_error(
          "Lusamine promoted Chaotic Swell after a required resource was spent.");
    }
  }
}

void test_missing_evolution_or_horizon_blocks_the_route() {
  {
    Fixture fixture;
    sim::State state = exact_state();
    state.active =
        sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
    state.hand.erase(
        std::remove(state.hand.begin(), state.hand.end(),
                    sim::Card::RegidragoVstar),
        state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

    // Legacy Star requires Regidrago VSTAR in play. A missing legal evolution
    // continuation cannot justify Lusamine:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1392
    if (!sim::EngineTestAccess::lusamine_targets(fixture.engine).empty()) {
      throw std::runtime_error(
          "Lusamine promoted Chaotic Swell without a VSTAR continuation.");
    }
  }

  {
    Fixture fixture(rule_box_scenario(2));
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());

    // The repository does not start a delayed line after the configured horizon:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1392
    if (!sim::EngineTestAccess::lusamine_targets(fixture.engine).empty()) {
      throw std::runtime_error(
          "Lusamine promoted Chaotic Swell beyond the configured horizon.");
    }
  }
}

void test_lusamine_still_requires_two_targets() {
  Fixture fixture;
  sim::State state = exact_state();
  state.discard = {sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Lusamine says to recover exactly two Supporter and/or Stadium cards. The
  // advancing Chaotic Swell target cannot make a one-target resolution legal:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/1392
  const std::vector<sim::Card> targets =
      sim::EngineTestAccess::lusamine_targets(fixture.engine);
  if (targets.size() != 1U || sim::EngineTestAccess::play_lusamine(fixture.engine)) {
    throw std::runtime_error(
        "Lusamine resolved without its mandatory second target.");
  }
}

void test_seed_20_reaches_t4_through_swell() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-rulebox-ability-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("The issue-1392 seed fixture is unavailable.");
  }

  std::mt19937_64 rng(20);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  if (outcome.first_ready_turn != 4) {
    throw std::runtime_error(
        "Seed 20 did not improve from setup failure to T4 readiness.");
  }

  const auto has_line = [&trace](const std::string& fragment) {
    return std::any_of(
        trace.lines.begin(), trace.lines.end(),
        [&fragment](const std::string& line) {
          return line.find(fragment) != std::string::npos;
        });
  };

  // The source-bound trace must recover and play Chaotic Swell, restore Legacy
  // Star, and retain the existing T4 strict-JIT finish:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/sm12-187
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1392
  if (!has_line("Recovered from discard: Chaotic Swell, Guzma") ||
      !has_line("Played Chaotic Swell, replacing the modeled Path") ||
      !has_line("LEGACY STAR") ||
      !has_line("T4 | READY")) {
    throw std::runtime_error(
        "Seed 20 lost the confirmed Chaotic Swell unlock trace.");
  }
}

}  // namespace

int main() {
  test_swell_is_the_advancing_first_target();
  test_no_lock_keeps_swell_fallback_only();
  test_used_resources_block_the_unlock_projection();
  test_missing_evolution_or_horizon_blocks_the_route();
  test_lusamine_still_requires_two_targets();
  test_seed_20_reaches_t4_through_swell();
}
