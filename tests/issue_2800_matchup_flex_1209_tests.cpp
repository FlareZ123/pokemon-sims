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
  }
  static bool t1_route_available(const Engine& engine) {
    return engine.issue_1209_t1_treasure_tapu_crispin_route_available();
  }
  static bool t2_route_available(const Engine& engine) {
    return engine.issue_1209_t2_treasure_tapu_context_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile dci) {
  return sim::Scenario{"issue-2800", dci, sim::LockMode::None, true, 5};
}

sim::State t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::Dragapult,
                sim::Card::RegidragoVstar, sim::Card::Arven};
  state.deck = {sim::Card::RegidragoV, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

sim::State t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dragapult,
                sim::Card::RegidragoVstar, sim::Card::Arven};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario(dci), recipe, rng, nullptr);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_t1_same_turn_jit_profile_parity() {
  // Both JIT profiles require payload discard on the readiness turn, while
  // NoDiscardControl has different discard timing and must not inherit this route:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2800
  std::mt19937_64 strict_rng{280001};
  sim::Engine strict = make_engine(sim::DciProfile::StrictJit, strict_rng, t1_state());
  expect(sim::EngineTestAccess::t1_route_available(strict),
         "StrictJit must retain the proven #1209 T1 route.");

  std::mt19937_64 flex_rng{280002};
  sim::Engine flex = make_engine(sim::DciProfile::MatchupFlexJit, flex_rng, t1_state());
  expect(sim::EngineTestAccess::t1_route_available(flex),
         "MatchupFlexJit must admit the same observable #1209 T1 route.");

  std::mt19937_64 control_rng{280003};
  sim::Engine control = make_engine(sim::DciProfile::NoDiscardControl, control_rng, t1_state());
  expect(!sim::EngineTestAccess::t1_route_available(control),
         "NoDiscardControl must remain outside the same-turn JIT route.");
}

void test_t2_same_turn_jit_profile_parity() {
  // The K1 T2 Treasure -> Tapu -> Crispin connector has identical legality under
  // StrictJit and MatchupFlexJit because both require the Dragon payload this turn:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2800
  std::mt19937_64 strict_rng{280004};
  sim::Engine strict = make_engine(sim::DciProfile::StrictJit, strict_rng, t2_state());
  expect(sim::EngineTestAccess::t2_route_available(strict),
         "StrictJit must retain the proven #1209 T2 route.");

  std::mt19937_64 flex_rng{280005};
  sim::Engine flex = make_engine(sim::DciProfile::MatchupFlexJit, flex_rng, t2_state());
  expect(sim::EngineTestAccess::t2_route_available(flex),
         "MatchupFlexJit must admit the same K1 #1209 T2 route.");

  std::mt19937_64 control_rng{280006};
  sim::Engine control = make_engine(sim::DciProfile::NoDiscardControl, control_rng, t2_state());
  expect(!sim::EngineTestAccess::t2_route_available(control),
         "NoDiscardControl must remain outside the same-turn JIT T2 route.");
}
}  // namespace

int main() {
  try {
    test_t1_same_turn_jit_profile_parity();
    test_t2_same_turn_jit_profile_parity();
    std::cout << "Issue 2800 MatchupFlex #1209 parity tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
