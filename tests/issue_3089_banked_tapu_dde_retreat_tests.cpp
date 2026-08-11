#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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
  static bool retreat_banked_tapu(Engine& engine) {
    return engine.retreat_banked_tapu_to_regidrago();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon vstar(const int grass, const int fire, const int dde) {
  sim::Pokemon pokemon{sim::Card::RegidragoVstar, 1, grass, fire,
                       sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

struct Fixture {
  sim::Scenario scenario{"issue-3089", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{3089};
  sim::Engine engine{scenario, recipe, rng};
};

void ready_dde_target_promotes(const int grass, const int fire,
                               const sim::Card retreat_payment) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1,
                              retreat_payment == sim::Card::Grass ? 1 : 0,
                              retreat_payment == sim::Card::Fire ? 1 : 0,
                              sim::Tool::None};
  state.bench.push_back(vstar(grass, fire, 1));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // DDE supplies two Energy of every type on a Dragon Pokemon, so DDE plus either
  // Basic Grass or Basic Fire already pays Regidrago VSTAR's GGF Apex Dragon cost.
  // Tapu Lele-GX may discard its attached Basic Energy to pay Retreat Cost 1:
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // DDE model: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3089
  expect(sim::EngineTestAccess::retreat_banked_tapu(fixture.engine),
         "Apex-ready DDE promotion target was rejected");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Regidrago VSTAR was not promoted");
  expect(after.retreat_used, "Retreat usage was not recorded");
  expect(std::find(after.discard.begin(), after.discard.end(), retreat_payment) !=
             after.discard.end(),
         "Tapu Retreat payment did not reach discard");
}

void incomplete_dde_target_stays_benched() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 1, 0,
                              sim::Tool::None};
  state.bench.push_back(vstar(0, 0, 1));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One DDE is only two physical Energy and cannot pay the three-Energy GGF cost:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/3089
  expect(!sim::EngineTestAccess::retreat_banked_tapu(fixture.engine),
         "Incomplete DDE-only Regidrago was promoted");
}

void stronger_free_retreat_still_dominates() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 1, 0,
                              sim::Tool::None};
  state.bench.push_back(vstar(1, 0, 1));
  state.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 1});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Latias ex provides the modeled free-retreat route, which remains preferred
  // over spending Tapu's banked Basic Energy:
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/3089
  expect(!sim::EngineTestAccess::retreat_banked_tapu(fixture.engine),
         "Paid Tapu Retreat preempted the free Latias route");
}

}  // namespace

int main() {
  try {
    ready_dde_target_promotes(1, 0, sim::Card::Grass);
    ready_dde_target_promotes(0, 1, sim::Card::Fire);
    incomplete_dde_target_stays_benched();
    stronger_free_retreat_still_dominates();
    std::cout << "Issue 3089 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
