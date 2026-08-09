#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool future_wonder_tag_target(Engine& engine) {
    return engine.future_turn_wonder_tag_route_has_live_target();
  }
  static bool needs_tapu(Engine& engine) {
    return engine.needs_tapu_connector();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon regidrago_v(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoV, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2446", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2446};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State opening_state() {
  sim::State state;
  state.turn = 1;
  state.manual_energy_used = true;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::RegidragoVstar};
  return state;
}

void test_dde_with_only_grass_searchable_banks_crispin() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = regidrago_v(0, 0, 1);
  state.deck = {sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // This is a legal going-first T1 state: Regidrago V began in play and the T1
  // manual attachment put DDE on it; the held VSTAR supplies the legal T2 evolution.
  // If Crispin finds only the sole Grass, the official ruling puts it into hand.
  // T2 then has a fresh manual attachment, so that Grass completes DDE + Grass.
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Evolution and manual attachment rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2446
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Wonder Tag rejected DDE plus sole searchable Grass Crispin line.");
}

void test_dde_with_only_fire_searchable_banks_crispin() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = regidrago_v(0, 0, 1);
  state.deck = {sim::Card::Crispin, sim::Card::Fire};
  state.prizes = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The sole Fire likewise goes to hand under the official one-card Crispin ruling,
  // then the fresh T2 manual attachment makes DDE + Fire pay Apex Dragon.
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Wonder Tag rejected DDE plus sole searchable Fire Crispin line.");
}

void test_basic_only_one_type_cannot_complete() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = regidrago_v(1, 0, 0);
  state.deck = {sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The sole Grass goes to hand and a T2 manual Grass still cannot satisfy Apex's
  // Fire requirement without DDE.
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Basic-only one-type Crispin line was incorrectly accepted.");
}

void test_basic_only_two_types_still_completes() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = regidrago_v(1, 0, 0);
  state.deck = {sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // On T2 Crispin can attach one of two searched Basic types and put the other into
  // hand for the fresh manual attachment, preserving the Basic-only completion route.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Manual Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Basic-only two-type Crispin route regressed.");
}

void test_pre_regidrago_tapu_trigger_is_preserved() {
  Fixture fixture;
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Klara,
                sim::Card::BrilliantBlender, sim::Card::MegaDragonite,
                sim::Card::FieldBlower, sim::Card::Arven,
                sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), false, false);

  // This public K0 state matches the seed-293 pre-search graph. The established
  // policy spends Tapu Lele-GX here so Wonder Tag can preserve the deterministic
  // Steven/FSS continuation instead of depending on an unknown next draw.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0/K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Existing regression contract: https://github.com/FlareZ123/pokemon-sims/issues/1022
  // Refined interaction scope: https://github.com/FlareZ123/pokemon-sims/issues/2446
  expect(sim::EngineTestAccess::needs_tapu(fixture.engine),
         "The DDE Crispin refinement must preserve the pre-Regidrago Tapu trigger.");
}

}  // namespace

int main() {
  try {
    test_dde_with_only_grass_searchable_banks_crispin();
    test_dde_with_only_fire_searchable_banks_crispin();
    test_basic_only_one_type_cannot_complete();
    test_basic_only_two_types_still_completes();
    test_pre_regidrago_tapu_trigger_is_preserved();
    std::cout << "Issue 2446 opening Tapu DDE Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
