#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct Debug1552ProfileAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = true;
  }

  static bool attach_manual(Engine& engine) {
    return engine.attach_manual();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State public_t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0};
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::DialgaGX,
      sim::Card::Serena,
      sim::Card::BrilliantBlender,
      sim::Card::QuickBall,
      sim::Card::Fire,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::Channeler,
  };
  state.prizes = {
      sim::Card::Arven,
      sim::Card::FieldBlower,
      sim::Card::Guzma,
      sim::Card::Klara,
      sim::Card::Lusamine,
      sim::Card::PathToPeak,
  };
  state.discard = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure};
  return state;
}

bool completes_same_public_route(const sim::DciProfile dci,
                                 const bool going_first,
                                 const std::uint64_t seed) {
  const sim::Scenario scenario{"debug-1552-profile", dci,
                               sim::LockMode::None, going_first, 5};
  std::mt19937_64 rng{seed};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::Debug1552ProfileAccess::set_state(engine, public_t2_state());
  sim::Debug1552ProfileAccess::attach_manual(engine);
  const sim::State& state = sim::Debug1552ProfileAccess::state(engine);
  const bool payload = std::find(state.discarded_this_turn.begin(),
                                 state.discarded_this_turn.end(),
                                 sim::Card::DialgaGX) != state.discarded_this_turn.end();
  return state.active && state.active->card == sim::Card::RegidragoVstar &&
         state.active->grass >= 2 && state.active->fire >= 1 && payload &&
         state.supporter_used && state.manual_energy_used;
}

void test_equivalent_public_state_is_profile_invariant() {
  // The exact physical K1 state and legal route are inherited from #1552/#1932.
  // Matchup-flex JIT still requires a Dragon payload on the actual ready turn,
  // so changing only the registered DCI profile or seat does not remove any
  // card/rule prerequisite of Quick Ball -> Tapu Lele-GX -> Crispin -> GGF:
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1552
  // K1 correction: https://github.com/FlareZ123/pokemon-sims/issues/1932
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  expect(completes_same_public_route(sim::DciProfile::StrictJit, true, 155201),
         "strict-jit/go-first control did not complete");
  expect(completes_same_public_route(sim::DciProfile::MatchupFlexJit, true, 155202),
         "equivalent matchup-flex/go-first public state was rejected");
  expect(completes_same_public_route(sim::DciProfile::StrictJit, false, 155203),
         "equivalent strict-jit/go-second public state was rejected");
}
}  // namespace

int main() {
  try {
    test_equivalent_public_state_is_profile_invariant();
    std::cout << "Issue-1552 profile-invariance debug audit passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
