#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
    return engine.issue_2272_route_replaced_arven_quick_ball_available();
  }

  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-2272/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2272};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State seed_456_t4_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::FieldBlower, sim::Card::Arven,
                sim::Card::PathToPeak, sim::Card::BrilliantBlender,
                sim::Card::MysteriousTreasure, sim::Card::QuickBall};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::StevensResolve, sim::Card::Crispin,
                   sim::Card::Serena, sim::Card::Dragapult};
  return state;
}

void test_seed_456_t4_admits_route_replaced_arven_cost() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, seed_456_t4_state());

  // K1 proves the exact current-turn completion. Quick Ball can legally discard
  // another hand card and search the Basic Latias ex. Skyliner then removes the
  // Basic Dialga-GX Retreat Cost, while held Brilliant Blender independently puts
  // a Dragon payload into discard for Apex Dragon during the same strict-JIT turn.
  // Arven's Item-search role is therefore replaced by the already-held Quick Ball.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, search, Ability, Retreat, and Supporter procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / strict-JIT / dynamic DCI / earliest-route policy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2272
  if (!sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("Known #2272 T4 completion route was rejected.");
  }
  if (!sim::EngineTestAccess::play_quick_ball(fixture.engine)) {
    throw std::runtime_error("#2272 Quick Ball route did not play.");
  }
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  if (std::find(state.discard.begin(), state.discard.end(), sim::Card::Arven) ==
      state.discard.end()) {
    throw std::runtime_error("#2272 Quick Ball did not spend route-replaced Arven.");
  }
  if (std::find(state.hand.begin(), state.hand.end(), sim::Card::LatiasEx) ==
      state.hand.end()) {
    throw std::runtime_error("#2272 Quick Ball did not search Latias ex.");
  }
}

void test_route_requires_k1_blender_and_known_payload() {
  Fixture fixture;
  sim::State state = seed_456_t4_state();
  sim::EngineTestAccess::set_state(fixture.engine, state, false);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route used exact deck identities while K0.");
  }

  state = seed_456_t4_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::BrilliantBlender), state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route spent Arven without held Blender.");
  }

  state = seed_456_t4_state();
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route spent Arven without a known deck payload.");
  }
}

void test_route_requires_complete_vstar_energy_and_active_position_axes() {
  Fixture fixture;
  sim::State state = seed_456_t4_state();
  state.bench.front().grass = 1;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route spent Arven before GGF was complete.");
  }

  state = seed_456_t4_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1, sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route fired when Active VSTAR was already solved.");
  }

  state = seed_456_t4_state();
  state.retreat_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route ignored the once-per-turn retreat gate.");
  }
}

void test_route_preserves_lower_dci_and_direct_payload_costs() {
  Fixture fixture;
  sim::State state = seed_456_t4_state();
  state.hand.push_back(sim::Card::BattleVipPass);
  sim::EngineTestAccess::set_state(fixture.engine, state);
  // Battle VIP Pass is dead after turn one and must remain a lower-DCI Quick Ball
  // cost than the still-live Supporter Arven.
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed regression boundary: https://github.com/FlareZ123/pokemon-sims/issues/2272
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route displaced a lower-DCI dead Item cost.");
  }

  state = seed_456_t4_state();
  state.hand.push_back(sim::Card::GoodraVstar);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route displaced a direct held strict-JIT payload cost.");
  }
}

void test_route_respects_search_bench_and_lock_gates() {
  Fixture fixture;
  sim::State state = seed_456_t4_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::LatiasEx),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route accepted without searchable Latias ex.");
  }

  state = seed_456_t4_state();
  for (int i = 0; i < 5; ++i) {
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine)) {
    throw std::runtime_error("#2272 route accepted with no Bench space.");
  }

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  sim::Scenario item_scenario{"issue-2272/item-lock", sim::DciProfile::StrictJit,
                              sim::LockMode::FullItem, false, 5};
  std::mt19937_64 item_rng{22721};
  sim::Engine item_engine{item_scenario, recipe, item_rng};
  sim::EngineTestAccess::set_state(item_engine, seed_456_t4_state());
  if (sim::EngineTestAccess::route_available(item_engine)) {
    throw std::runtime_error("#2272 route accepted through Item lock.");
  }

  sim::Scenario ability_scenario{"issue-2272/ability-lock", sim::DciProfile::StrictJit,
                                 sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 ability_rng{22722};
  sim::Engine ability_engine{ability_scenario, recipe, ability_rng};
  sim::EngineTestAccess::set_state(ability_engine, seed_456_t4_state());
  // Latias ex is a Rule Box Pokémon, so the modeled Path-style lock suppresses
  // Skyliner and removes this promotion route.
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Repository lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-semantics
  // Confirmed regression boundary: https://github.com/FlareZ123/pokemon-sims/issues/2272
  if (sim::EngineTestAccess::route_available(ability_engine)) {
    throw std::runtime_error("#2272 route accepted through Rule Box Ability lock.");
  }
}

}  // namespace

int main() {
  test_seed_456_t4_admits_route_replaced_arven_cost();
  test_route_requires_k1_blender_and_known_payload();
  test_route_requires_complete_vstar_energy_and_active_position_axes();
  test_route_preserves_lower_dci_and_direct_payload_costs();
  test_route_respects_search_bench_and_lock_gates();
}
