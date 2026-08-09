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

  static bool issue_2270_route(const Engine& engine) {
    return engine.issue_2270_wonder_tag_arven_latias_route_available();
  }

  static bool needs_tapu(Engine& engine) {
    return engine.needs_tapu_connector();
  }
};
}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-2270/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2270};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State seed_152_route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::TapuLeleGX, sim::Card::GoodraVstar};
  state.deck = {sim::Card::Arven, sim::Card::QuickBall, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_quick_ball_latias_can_complete_the_missing_active_axis() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, seed_152_route_state());

  // The K1 state has an Apex-powered Benched Regidrago VSTAR, a held Dragon
  // payload, and known Arven -> Quick Ball -> Latias ex access. Quick Ball may
  // discard Goodra, search Basic Latias ex, and Skyliner gives Basic Dialga-GX
  // free Retreat. The late Tapu preflight must therefore admit Wonder Tag.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, Item, discard, search, Bench, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
  if (!sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error(
        "K1 Wonder Tag -> Arven -> Quick Ball -> Latias promotion route was rejected.");
  }
  if (!sim::EngineTestAccess::needs_tapu(fixture.engine)) {
    throw std::runtime_error(
        "The live #2270 route did not reach the active Tapu connector preflight.");
  }
}

void test_double_dragon_energy_semantically_pays_apex_for_the_route() {
  Fixture fixture;
  sim::State state = seed_152_route_state();
  state.bench[0].grass = 1;
  state.bench[0].fire = 0;
  state.bench[0].double_dragon = 1;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Double Dragon Energy provides two Energy of every type while attached to a
  // Dragon Pokemon, so one DDE plus one Grass semantically pays Apex Dragon's GGF.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Repository DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
  if (!sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error(
        "The #2270 route rejected semantically complete DDE + Grass Apex payment.");
  }
}

void test_route_stays_live_after_tapu_occupies_its_bench_slot() {
  Fixture fixture;
  sim::State state = seed_152_route_state();
  state.hand = {sim::Card::GoodraVstar};
  state.bench.push_back(
      sim::Pokemon{sim::Card::TapuLeleGX, 3, 0, 0, sim::Tool::None});
  state.bench.push_back(
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Wonder Tag resolves after Tapu Lele-GX has already entered the Bench. One
  // remaining slot is therefore sufficient for Latias ex.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Bench procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
  if (!sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error(
        "The Latias continuation died after Wonder Tag consumed Tapu's Bench slot.");
  }
}

void test_route_requires_every_legal_completion_gate() {
  Fixture fixture;

  sim::State state = seed_152_route_state();
  state.active = sim::Pokemon{sim::Card::Dragapult, 0, 0, 0, sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error("Skyliner route accepted a non-Basic Active Pokemon.");
  }

  state = seed_152_route_state();
  state.hand = {sim::Card::TapuLeleGX};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error("Quick Ball route accepted without a held Dragon payload cost.");
  }

  state = seed_152_route_state();
  state.deck = {sim::Card::Arven, sim::Card::QuickBall, sim::Card::Grass,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error("Quick Ball route accepted without a known Latias ex target.");
  }

  state = seed_152_route_state();
  state.retreat_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error("Latias route accepted after the once-per-turn Retreat was spent.");
  }

  state = seed_152_route_state();
  state.bench.push_back(
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  state.bench.push_back(
      sim::Pokemon{sim::Card::Pineco, 1, 0, 0, sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::issue_2270_route(fixture.engine)) {
    throw std::runtime_error("Pre-Wonder-Tag route accepted with only one open Bench slot.");
  }
}

void test_route_respects_item_and_rule_box_ability_locks() {
  sim::State state = seed_152_route_state();

  sim::Scenario item_scenario{"issue-2270/item-lock", sim::DciProfile::StrictJit,
                              sim::LockMode::FullItem, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 item_rng{22701};
  sim::Engine item_engine{item_scenario, recipe, item_rng};
  sim::EngineTestAccess::set_state(item_engine, state);
  if (sim::EngineTestAccess::issue_2270_route(item_engine)) {
    throw std::runtime_error("Wonder Tag-Arven route accepted Quick Ball through Item lock.");
  }

  sim::Scenario ability_scenario{"issue-2270/ability-lock", sim::DciProfile::StrictJit,
                                 sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 ability_rng{22702};
  sim::Engine ability_engine{ability_scenario, recipe, ability_rng};
  sim::EngineTestAccess::set_state(ability_engine, std::move(state));

  // Tapu Lele-GX and Latias ex are Rule Box Pokemon, so a Path-style Rule Box
  // Ability lock suppresses Wonder Tag and Skyliner in this modeled scenario.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Repository lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-semantics
  // Confirmed regression boundary: https://github.com/FlareZ123/pokemon-sims/issues/2270
  if (sim::EngineTestAccess::issue_2270_route(ability_engine)) {
    throw std::runtime_error("Wonder Tag-Arven-Latias route accepted through Rule Box Ability lock.");
  }
}

}  // namespace

int main() {
  test_quick_ball_latias_can_complete_the_missing_active_axis();
  test_double_dragon_energy_semantically_pays_apex_for_the_route();
  test_route_stays_live_after_tapu_occupies_its_bench_slot();
  test_route_requires_every_legal_completion_gate();
  test_route_respects_item_and_rule_box_ability_locks();
}
