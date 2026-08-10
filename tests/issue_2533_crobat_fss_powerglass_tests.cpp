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
  static bool fss_crobat_compression_available(Engine& engine) {
    return engine.fss_crobat_compression_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon active_regidrago(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoV, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2533", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2533};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = active_regidrago(0, 0, 0);
  state.hand = {sim::Card::ForestSealStone, sim::Card::CrobatV,
                sim::Card::Powerglass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_paid_dde_grass_does_not_reserve_powerglass() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(1, 0, 1);
  state.discard = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // DDE + Grass already pays Apex Dragon; recovering Fire cannot improve it.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2533
  expect(sim::EngineTestAccess::fss_crobat_compression_available(fixture.engine),
         "DDE + Grass incorrectly reserved the Tool slot for redundant Powerglass.");
}

void test_paid_dde_fire_does_not_reserve_powerglass() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(0, 1, 1);
  state.discard = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2533
  expect(sim::EngineTestAccess::fss_crobat_compression_available(fixture.engine),
         "DDE + Fire incorrectly reserved the Tool slot for redundant Powerglass.");
}

void test_dde_only_finishing_basic_keeps_powerglass_live() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(0, 0, 1);
  state.discard = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // DDE contributes two units and recovered Grass is the third Apex unit.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2533
  expect(!sim::EngineTestAccess::fss_crobat_compression_available(fixture.engine),
         "DDE-only attacker failed to reserve live Powerglass for a finishing Basic.");
}

void test_basic_only_incomplete_attacker_keeps_powerglass_live() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(1, 0, 0);
  state.discard = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // Recovering Fire advances a genuinely incomplete Basic-only Apex state.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2533
  expect(!sim::EngineTestAccess::fss_crobat_compression_available(fixture.engine),
         "Basic-only incomplete attacker lost a genuinely progressive Powerglass hold.");
}
}  // namespace

int main() {
  try {
    test_paid_dde_grass_does_not_reserve_powerglass();
    test_paid_dde_fire_does_not_reserve_powerglass();
    test_dde_only_finishing_basic_keeps_powerglass_live();
    test_basic_only_incomplete_attacker_keeps_powerglass_live();
    std::cout << "Issue 2533 Crobat/FSS Powerglass tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
