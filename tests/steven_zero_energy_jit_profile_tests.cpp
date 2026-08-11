#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true; // K1 prize deduction after deck inspection: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  }

  static bool steven_zero_energy_latias_vessel_candidate(const Engine& engine) {
    return engine.steven_zero_energy_latias_vessel_candidate();
  }
};

}  // namespace sim

namespace {

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Crispin, sim::Card::LatiasEx,
                sim::Card::EarthenVessel, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool candidate_for(const sim::DciProfile dci) {
  // The route is physically identical across profiles. Steven banks Crispin, Latias ex,
  // and Earthen Vessel; Vessel pays its printed discard cost with the held Dragon on
  // the same ready turn, so both same-ready-turn JIT profiles satisfy the policy:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3208
  const sim::Scenario scenario{"issue-3208-steven-semantic-jit", dci,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3208};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_known_state(engine, route_state());
  return sim::EngineTestAccess::steven_zero_energy_latias_vessel_candidate(engine);
}

void test_same_ready_turn_jit_profiles_match() {
  if (!candidate_for(sim::DciProfile::StrictJit)) {
    throw std::runtime_error("StrictJit must admit the complete Steven-Latias-Vessel route.");
  }
  if (!candidate_for(sim::DciProfile::MatchupFlexJit)) {
    throw std::runtime_error("MatchupFlexJit must share StrictJit's ready-turn payload admission.");
  }
  if (candidate_for(sim::DciProfile::NoDiscardControl)) {
    throw std::runtime_error("NoDiscardControl must remain outside the same-ready-turn JIT contract.");
  }
}

}  // namespace

int main() {
  try {
    test_same_ready_turn_jit_profiles_match();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
