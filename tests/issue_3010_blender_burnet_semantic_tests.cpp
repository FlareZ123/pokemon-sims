#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool hold_blender(const Engine& engine) {
    return engine.issue_1646_hold_blender_for_burnet_finish_visible();
  }
  static bool play_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  explicit Fixture(const sim::DciProfile profile)
      : scenario{"issue-3010/exact", profile, sim::LockMode::None, true, 5},
        recipe{sim::baseline_recipe()},
        rng{3010},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State burnet_owned_payload_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {
      sim::Card::ProfessorBurnet,
      sim::Card::BrilliantBlender,
      sim::Card::Grass,
  };
  state.deck = {sim::Card::Dragapult};
  return state;
}

void install(Fixture& fixture, sim::State state) {
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
}

void test_same_turn_jit_profiles_preserve_blender() {
  for (const sim::DciProfile profile : {
           sim::DciProfile::StrictJit,
           sim::DciProfile::MatchupFlexJit,
       }) {
    Fixture fixture{profile};
    install(fixture, burnet_owned_payload_state());

    // Burnet searches and discards up to two cards, both JIT profiles require the
    // Dragon payload this turn, and one held Grass completes Apex independently.
    // The state is deliberately T4 with no Vessel/Quick Ball discard breadcrumbs,
    // proving the choice is based on route semantics rather than witness identity:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter and Energy rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-turn JIT and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/3010
    expect(sim::EngineTestAccess::hold_blender(fixture.engine),
           "A same-turn-JIT profile failed to preserve Blender for held Burnet.");
    expect(!sim::EngineTestAccess::play_blender(fixture.engine),
           "Blender was spent on the payload axis already owned by Burnet.");
    expect(std::count(sim::EngineTestAccess::state(fixture.engine).hand.begin(),
                      sim::EngineTestAccess::state(fixture.engine).hand.end(),
                      sim::Card::BrilliantBlender) == 1,
           "The singleton Blender did not remain in hand.");
  }
}

void test_non_jit_profile_is_not_forced_into_hold() {
  Fixture fixture{sim::DciProfile::NoDiscardControl};
  install(fixture, burnet_owned_payload_state());

  // No-discard-control does not impose the same current-turn payload timing, so
  // this #3010-specific hold must stay inside the documented JIT profiles:
  // DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  expect(!sim::EngineTestAccess::hold_blender(fixture.engine),
         "The same-turn-JIT hold leaked into no-discard-control.");
}

void test_burnet_illegal_releases_blender() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit};
  sim::State state = burnet_owned_payload_state();
  state.supporter_used = true;
  install(fixture, std::move(state));

  // Only one Supporter may be used during a turn, so a spent Supporter action
  // means held Burnet no longer owns the payload axis:
  // Advanced Supporter rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/3010
  expect(!sim::EngineTestAccess::hold_blender(fixture.engine),
         "Blender was held after the Supporter action was spent.");
}

void test_missing_payload_releases_blender() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit};
  sim::State state = burnet_owned_payload_state();
  state.deck.clear();
  install(fixture, std::move(state));

  // Burnet cannot establish the modeled payload axis when K1 proves no permitted
  // Dragon payload remains in the deck:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(!sim::EngineTestAccess::hold_blender(fixture.engine),
         "Blender was held without a Burnet-searchable payload.");
}

void test_unresolved_energy_axis_releases_blender() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit};
  sim::State state = burnet_owned_payload_state();
  state.active->fire = 0;
  install(fixture, std::move(state));

  // One held Grass cannot turn a G-only Active into GGF, so Burnet does not own a
  // complete ready route and Blender remains available for another unresolved axis:
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Manual Energy attachment: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/3010
  expect(!sim::EngineTestAccess::hold_blender(fixture.engine),
         "Blender was held while the Energy axis could not complete.");
}

}  // namespace

int main() {
  try {
    test_same_turn_jit_profiles_preserve_blender();
    test_non_jit_profile_is_not_forced_into_hold();
    test_burnet_illegal_releases_blender();
    test_missing_payload_releases_blender();
    test_unresolved_energy_axis_releases_blender();
    std::cout << "Issue 3010 semantic Blender/Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
