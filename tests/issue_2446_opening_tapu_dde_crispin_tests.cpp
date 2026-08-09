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
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon vstar(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire, sim::Tool::None};
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
  state.hand = {sim::Card::TapuLeleGX};
  return state;
}

void test_dde_with_only_grass_searchable_banks_crispin() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(0, 0, 1);
  state.deck = {sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The T1 manual attachment was already spent on DDE. Wonder Tag can still bank
  // Crispin for T2; Crispin searches the sole available Grass into hand, then the
  // fresh T2 manual attachment makes DDE + Grass pay Apex Dragon.
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2446
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Wonder Tag rejected DDE plus sole searchable Grass Crispin line.");
}

void test_dde_with_only_fire_searchable_banks_crispin() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(0, 0, 1);
  state.deck = {sim::Card::Crispin, sim::Card::Fire};
  state.prizes = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Either Basic type is a legal final unit beside DDE on a Dragon.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Wonder Tag rejected DDE plus sole searchable Fire Crispin line.");
}

void test_basic_only_one_type_cannot_complete() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(1, 0, 0);
  state.deck = {sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One additional Grass cannot satisfy Apex Dragon's Fire requirement without DDE.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Basic-only one-type Crispin line was incorrectly accepted.");
}

void test_basic_only_two_types_still_completes() {
  Fixture fixture;
  sim::State state = opening_state();
  state.active = vstar(1, 0, 0);
  state.deck = {sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // On T2 Crispin can attach one searched type and put the other into hand for the
  // fresh manual attachment, preserving the pre-DDE Basic-only completion route.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Manual Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::future_wonder_tag_target(fixture.engine),
         "Basic-only two-type Crispin route regressed.");
}

}  // namespace

int main() {
  try {
    test_dde_with_only_grass_searchable_banks_crispin();
    test_dde_with_only_fire_searchable_banks_crispin();
    test_basic_only_one_type_cannot_complete();
    test_basic_only_two_types_still_completes();
    std::cout << "Issue 2446 opening Tapu DDE Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
