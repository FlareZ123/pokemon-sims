#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct Issue2773TestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_2153_steven_latias_blender_route_available();
  }
  static bool play_route(Engine& engine) {
    return engine.play_issue_2153_steven_latias_blender_route();
  }
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

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::DialgaGX,
      sim::Card::Dragapult,
      sim::Card::StevensResolve,
      sim::Card::EarthenVessel,
      sim::Card::Crispin,
      sim::Card::ForestSealStone,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::LatiasEx,
      sim::Card::BrilliantBlender,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
      sim::Card::MysteriousTreasure,
      sim::Card::QuickBall,
  };
  state.prizes = {
      sim::Card::GoodraVstar,
      sim::Card::Channeler,
      sim::Card::TeamYellsCheer,
      sim::Card::QuickBall,
      sim::Card::Powerglass,
      sim::Card::FieldBlower,
  };
  return state;
}

sim::Scenario scenario(const sim::DciProfile dci) {
  return sim::Scenario{"issue-2773", dci, sim::LockMode::None, false, 5};
}

void issue_2153_route_uses_shared_jit_timing() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 strict_rng(2773);
  std::mt19937_64 flex_rng(2773);
  std::mt19937_64 control_rng(2773);
  sim::Engine strict_engine(scenario(sim::DciProfile::StrictJit), recipe,
                            strict_rng);
  sim::Engine flex_engine(scenario(sim::DciProfile::MatchupFlexJit), recipe,
                          flex_rng);
  sim::Engine control_engine(scenario(sim::DciProfile::NoDiscardControl), recipe,
                             control_rng);
  sim::Issue2773TestAccess::set_state(strict_engine, route_state());
  sim::Issue2773TestAccess::set_state(flex_engine, route_state());
  sim::Issue2773TestAccess::set_state(control_engine, route_state());

  // The exact #2153 K1 route banks Regidrago V, Latias ex, and Brilliant
  // Blender with Steven. Blender supplies the Dragon payload during the T3 ready
  // turn, which is the shared timing requirement of StrictJit and MatchupFlexJit.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Oricorio / Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-55 https://api.pokemontcg.io/v2/cards/sm2-60
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/2153
  // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2773
  expect(sim::Issue2773TestAccess::route_available(strict_engine),
         "StrictJit lost the issue-2153 Steven-to-Blender route.");
  expect(sim::Issue2773TestAccess::route_available(flex_engine),
         "MatchupFlexJit still rejects the issue-2153 Steven-to-Blender route.");
  expect(!sim::Issue2773TestAccess::route_available(control_engine),
         "NoDiscardControl incorrectly entered the same-turn JIT route.");

  expect(sim::Issue2773TestAccess::play_route(flex_engine),
         "MatchupFlexJit could not execute the admitted issue-2153 route.");
  const sim::State& after = sim::Issue2773TestAccess::state(flex_engine);
  expect(after.supporter_used && after.turn_ended,
         "The flex route did not consume Steven and end T1.");
  expect(contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.hand, sim::Card::LatiasEx) &&
             contains(after.hand, sim::Card::BrilliantBlender),
         "Steven did not bank the issue-2153 Regidrago V, Latias ex, Blender package.");
}
}  // namespace

int main() {
  try {
    issue_2153_route_uses_shared_jit_timing();
    std::cout << "Issue 2773 shared-JIT route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
