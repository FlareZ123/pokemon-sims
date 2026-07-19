#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool play_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

sim::Engine make_engine() {
  const sim::Scenario scenario{"issue-1031", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1031};
  return sim::Engine{scenario, recipe, rng};
}

sim::State latias_promotion_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_energy_incomplete_target_holds_blender() {
  sim::Engine engine = make_engine();
  sim::State state = latias_promotion_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Skyliner solves the Basic Active's Retreat Cost, while the selected Benched
  // Regidrago VSTAR still lacks Grass. The one-use ACE SPEC must remain held until
  // that exact attacker can pay Apex Dragon's GGF cost in the payload turn:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1031
  if (sim::EngineTestAccess::play_blender(engine)) {
    throw std::runtime_error("Blender spent before the promoted VSTAR could reach GGF");
  }
  if (std::find(sim::EngineTestAccess::state(engine).hand.begin(),
                sim::EngineTestAccess::state(engine).hand.end(),
                sim::Card::BrilliantBlender) ==
      sim::EngineTestAccess::state(engine).hand.end()) {
    throw std::runtime_error("Rejected route did not preserve Brilliant Blender");
  }
}

void test_final_manual_energy_keeps_route_live() {
  sim::Engine engine = make_engine();
  sim::State state = latias_promotion_state();
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The unused manual attachment can supply the final Grass before Skyliner promotes
  // the attacker, so Blender remains a productive same-turn payload outlet:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1031
  if (!sim::EngineTestAccess::play_blender(engine)) {
    throw std::runtime_error("Legal Latias plus final-Energy route was rejected");
  }
}

}  // namespace

int main() {
  test_energy_incomplete_target_holds_blender();
  test_final_manual_energy_keeps_route_live();
  std::cout << "Issue 1031 Blender-Latias Energy tests passed\n";
  return 0;
}
