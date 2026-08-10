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
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile dci) {
  return sim::Scenario{"issue-2764-jit-profile", dci, sim::LockMode::None,
                       true, 4};
}

sim::State exact_state() {
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

void same_turn_jit_profiles_share_the_route() {
  // Both JIT profiles require the Dragon payload on the ready turn:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Steven's Resolve banks the two missing cards and ends T3: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender supplies the T4 deck payload: https://api.pokemontcg.io/v2/cards/sv8-164
  // Apex Dragon costs GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, attachment, evolution, Item, search, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1798
  // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2764
  for (const sim::DciProfile dci : {sim::DciProfile::StrictJit,
                                    sim::DciProfile::MatchupFlexJit}) {
    sim::Scenario selected = scenario(dci);
    sim::DeckRecipe recipe = sim::baseline_recipe();
    std::mt19937_64 rng{2764};
    sim::Engine engine{selected, recipe, rng};
    sim::EngineTestAccess::set_state(engine, exact_state());

    expect(sim::EngineTestAccess::route_available(engine),
           "A same-turn JIT profile rejected the exact Steven-Blender route");
    expect(sim::EngineTestAccess::start_route(engine),
           "A same-turn JIT profile failed to start the Steven route");

    sim::State& state = sim::EngineTestAccess::state(engine);
    state.turn = 4;
    state.turn_ended = false;
    state.supporter_used = false;
    state.manual_energy_used = false;
    state.discarded_this_turn.clear();

    expect(sim::EngineTestAccess::finish_route(engine),
           "A same-turn JIT profile failed the committed Blender finish");
    expect(state.active && state.active->card == sim::Card::RegidragoVstar &&
               state.active->grass == 2 && state.active->fire == 2,
           "The shared JIT route did not evolve and reach GGFF");
  }
}

void no_discard_control_stays_outside_the_jit_route() {
  // No-discard-control permits earlier payload banking and remains a separate policy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed scope: https://github.com/FlareZ123/pokemon-sims/issues/2764
  sim::Scenario selected = scenario(sim::DciProfile::NoDiscardControl);
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{2764};
  sim::Engine engine{selected, recipe, rng};
  sim::EngineTestAccess::set_state(engine, exact_state());

  expect(!sim::EngineTestAccess::route_available(engine),
         "No-discard-control entered the same-turn-JIT-specific route");
}

}  // namespace

int main() {
  try {
    same_turn_jit_profiles_share_the_route();
    no_discard_control_stays_outside_the_jit_route();
    std::cout << "issue 2764 JIT profile tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "issue 2764 JIT profile tests failed: " << error.what() << '\n';
    return 1;
  }
}
