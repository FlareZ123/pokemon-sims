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
  static bool issue_1209_context(const Engine& engine) {
    return engine.issue_1209_t2_treasure_tapu_context_available();
  }
  static bool issue_1209_completion(const Engine& engine) {
    return engine.issue_1209_t2_treasure_tapu_crispin_completion_available();
  }
  static bool issue_1235_context(const Engine& engine) {
    return engine.issue_1235_t2_treasure_tapu_context_available();
  }
  static bool issue_1235_completion(const Engine& engine) {
    return engine.issue_1235_t2_treasure_tapu_crispin_completion_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon regi(const sim::Card card, const int grass, const int fire,
                  const int dde) {
  sim::Pokemon result{card, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2431", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2431};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State common_k1_state() {
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_1209_benched_dde_only_context_and_completion() {
  Fixture fixture;
  sim::State state = common_k1_state();
  state.hand.push_back(sim::Card::Arven);
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {regi(sim::Card::RegidragoV, 0, 0, 1)};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The inspected deck contains both different Basic Energy types, so Crispin may
  // legally put one into hand and attach the other. DDE supplies two units of every
  // type on the Dragon Regidrago, and either searched Basic attachment supplies the
  // third unit required by Apex Dragon. Mysterious Treasure can therefore pay the
  // held Dragon payload and search Tapu Lele-GX for the same-turn Wonder Tag line.
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Crispin: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/sv07/133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2431
  expect(sim::EngineTestAccess::issue_1209_context(fixture.engine),
         "#1209 must admit a prior-turn Benched DDE-only Regidrago.");
  expect(sim::EngineTestAccess::issue_1209_completion(fixture.engine),
         "#1209 shadow route must complete DDE plus Crispin Basic semantically.");
}

void test_1235_active_dde_only_context_and_completion() {
  Fixture fixture;
  sim::State state = common_k1_state();
  state.active = regi(sim::Card::RegidragoV, 0, 0, 1);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Both Basic types remain searchable, satisfying Crispin's different-type search
  // requirement. The exact shadow must judge the resulting attacker by semantic
  // Apex payment so DDE + either attached Basic is accepted.
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2431
  expect(sim::EngineTestAccess::issue_1235_context(fixture.engine),
         "#1235 must admit a prior-turn Active DDE-only Regidrago.");
  expect(sim::EngineTestAccess::issue_1235_completion(fixture.engine),
         "#1235 shadow route must accept semantic DDE Apex payment.");
}

void test_original_basic_energy_controls_remain_live() {
  Fixture fixture_1209;
  sim::State benched = common_k1_state();
  benched.hand.push_back(sim::Card::Arven);
  benched.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  benched.bench = {regi(sim::Card::RegidragoV, 1, 0, 0)};
  sim::EngineTestAccess::set_state(fixture_1209.engine, std::move(benched));
  expect(sim::EngineTestAccess::issue_1209_context(fixture_1209.engine),
         "#1209 original one-Grass staging control regressed.");

  Fixture fixture_1235;
  sim::State active = common_k1_state();
  active.active = regi(sim::Card::RegidragoV, 1, 0, 0);
  sim::EngineTestAccess::set_state(fixture_1235.engine, std::move(active));
  expect(sim::EngineTestAccess::issue_1235_context(fixture_1235.engine),
         "#1235 original one-Grass staging control regressed.");

  // These controls preserve the pre-DDE connector contract while the new states
  // extend it only through physical Energy attachment projection.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Repository DDE specification: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2431
}

void test_single_searchable_basic_is_rejected_for_crispin_attachment_route() {
  Fixture fixture;
  sim::State state = common_k1_state();
  state.active = regi(sim::Card::RegidragoV, 0, 0, 1);
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // When only one Basic Energy is found, the ruling puts that card into hand.
  // This same-turn route depends on Crispin's effect attachment, so one searchable
  // Basic is insufficient here even though a later manual attachment could use it.
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Crispin: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/sv07/133
  // Refined legality scope: https://github.com/FlareZ123/pokemon-sims/issues/2431
  expect(!sim::EngineTestAccess::issue_1235_context(fixture.engine),
         "#1235 admitted a Crispin-attachment route with only one searchable Basic.");
}
}  // namespace

int main() {
  try {
    test_1209_benched_dde_only_context_and_completion();
    test_1235_active_dde_only_context_and_completion();
    test_original_basic_energy_controls_remain_live();
    test_single_searchable_basic_is_rejected_for_crispin_attachment_route();
    std::cout << "issue_2431_dde_treasure_tapu_crispin_tests: all checks passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
