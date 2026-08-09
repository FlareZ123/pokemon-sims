#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_2271_surplus_regidrago_v_treasure_route_available();
  }
};
}  // namespace sim

namespace {
sim::State route_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::ForestSealStone};
  state.bench = {
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                sim::Card::DialgaGX, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Arven,
                sim::Card::EarthenVessel, sim::Card::Grass,
                sim::Card::Fire};
  return state;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(std::mt19937_64& rng) {
  sim::Scenario scenario{"issue-2271/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  return sim::Engine{scenario, sim::baseline_recipe(), rng};
}

void test_exact_route_and_boundaries() {
  std::mt19937_64 rng{2271};
  auto engine = make_engine(rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // The Active VSTAR already owns the Basic/evolution/Active axes. K1 proves
  // Treasure -> Tapu -> Arven -> Vessel -> Grass; Vessel can spend the separate
  // held Dialga-GX as the same-turn payload while the extra Regidrago V is DCI-high.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Ability, Supporter, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / strict-JIT / dynamic DCI / earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Reclaimed confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2271
  expect(sim::EngineTestAccess::route_available(engine),
         "issue-2271 exact route was rejected");

  auto state = route_state();
  sim::EngineTestAccess::set_state(engine, state, false);
  expect(!sim::EngineTestAccess::route_available(engine),
         "issue-2271 used exact deck identities in K0");

  state = route_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "issue-2271 fired without a separate current-turn payload");

  state = route_state();
  state.supporter_used = true;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "issue-2271 fired after the Supporter slot was spent");

  state = route_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "issue-2271 fired after the manual attachment was spent");

  state = route_state();
  state.deck.erase(state.deck.begin());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_available(engine),
         "issue-2271 fired without searchable Tapu Lele-GX");
}
}  // namespace

int main() {
  test_exact_route_and_boundaries();
  return 0;
}
