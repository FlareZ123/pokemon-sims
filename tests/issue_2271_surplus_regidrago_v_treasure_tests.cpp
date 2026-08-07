#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <optional>
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
    return engine.issue_2271_surplus_regidrago_v_treasure_route_available(
        Card::MysteriousTreasure);
  }

  static std::optional<Card> treasure_cost(const Engine& engine) {
    return engine.choose_discard(false, false, true, Card::MysteriousTreasure,
                                 false);
  }
};
}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-2271/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2271};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State seed_628_route_state() {
  sim::State state;
  state.turn = 4;
  state.active =
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::ForestSealStone};
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

void test_surplus_regidrago_v_unlocks_known_treasure_tapu_arven_vessel_finish() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, seed_628_route_state());

  // The Active Regidrago VSTAR already owns the Basic/evolution/Active axes. K1
  // proves Treasure -> Tapu -> Arven -> Vessel -> Grass; Vessel can discard the
  // separate held Dialga-GX on this turn, then Grass attachment completes GGF.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, search, discard, Ability, Supporter, and attachment procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / strict-JIT / dynamic DCI / earliest-route policy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2271
  if (!sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("Known #2271 completion route was rejected.");
  }
  const auto cost = sim::EngineTestAccess::treasure_cost(fixture.engine);
  if (!cost || *cost != sim::Card::RegidragoV) {
    throw std::runtime_error("Treasure did not admit the route-surplus Regidrago V.");
  }
}

void test_route_requires_k1_and_a_separate_vessel_payload() {
  Fixture fixture;
  sim::State state = seed_628_route_state();
  sim::EngineTestAccess::set_state(fixture.engine, state, false);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route used exact deck targets while still K0.");
  }

  state = seed_628_route_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route spent Regidrago V without a separate payload.");
  }
}

void test_route_requires_established_active_vstar_and_remaining_grass_axis() {
  Fixture fixture;
  sim::State state = seed_628_route_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route spent the Basic before the VSTAR axis was complete.");
  }

  state = seed_628_route_state();
  state.active =
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::ForestSealStone};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route fired after GGF was already complete.");
  }
}

void test_route_requires_every_search_and_action_gate() {
  Fixture fixture;

  sim::State state = seed_628_route_state();
  state.deck = {sim::Card::Arven, sim::Card::EarthenVessel, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted without searchable Tapu Lele-GX.");
  }

  state = seed_628_route_state();
  state.deck = {sim::Card::TapuLeleGX, sim::Card::EarthenVessel, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted without searchable Arven.");
  }

  state = seed_628_route_state();
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Arven, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted without searchable Earthen Vessel.");
  }

  state = seed_628_route_state();
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Arven, sim::Card::EarthenVessel};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted without searchable Grass Energy.");
  }

  state = seed_628_route_state();
  state.supporter_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted after the Supporter action was spent.");
  }

  state = seed_628_route_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted after manual attachment was spent.");
  }

  state = seed_628_route_state();
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2271 route accepted with a full Bench.");
  }
}

void test_route_respects_item_and_rule_box_ability_locks() {
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  sim::State state = seed_628_route_state();

  sim::Scenario item_scenario{"issue-2271/item-lock", sim::DciProfile::StrictJit,
                              sim::LockMode::FullItem, false, 5};
  std::mt19937_64 item_rng{22711};
  sim::Engine item_engine{item_scenario, recipe, item_rng};
  sim::EngineTestAccess::set_state(item_engine, state);
  if (sim::EngineTestAccess::route_available(item_engine)) {
    throw std::runtime_error("#2271 route accepted through Item lock.");
  }

  sim::Scenario ability_scenario{"issue-2271/ability-lock", sim::DciProfile::StrictJit,
                                 sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 ability_rng{22712};
  sim::Engine ability_engine{ability_scenario, recipe, ability_rng};
  sim::EngineTestAccess::set_state(ability_engine, std::move(state));
  // Tapu Lele-GX is a Rule Box Pokemon, so a Path-style modeled lock suppresses
  // Wonder Tag and blocks the route.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Repository lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-semantics
  // Confirmed regression boundary: https://github.com/FlareZ123/pokemon-sims/issues/2271
  if (sim::EngineTestAccess::route_available(ability_engine)) {
    throw std::runtime_error("#2271 route accepted through Rule Box Ability lock.");
  }
}

}  // namespace

int main() {
  test_surplus_regidrago_v_unlocks_known_treasure_tapu_arven_vessel_finish();
  test_route_requires_k1_and_a_separate_vessel_payload();
  test_route_requires_established_active_vstar_and_remaining_grass_axis();
  test_route_requires_every_search_and_action_gate();
  test_route_respects_item_and_rule_box_ability_locks();
}
