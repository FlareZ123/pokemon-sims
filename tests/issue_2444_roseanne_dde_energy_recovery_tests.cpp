#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
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
  static std::optional<Card> roseanne_finisher(const Engine& engine) {
    return engine.roseanne_finishing_basic_to_recover();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon attacker(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire,
                      sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2444", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2444};
  sim::Engine engine{scenario, recipe, rng};
};

void test_dde_finisher(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 0, 1);
  state.discard = {basic};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Roseanne restores the otherwise unavailable Basic, Vessel searches it, and
  // the unused manual attachment completes Apex Dragon alongside one DDE.
  // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
  expect(sim::EngineTestAccess::roseanne_finisher(fixture.engine) == basic,
         "Roseanne failed to recognize a DDE one-Basic completion.");
}

void test_basic_only_control() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(1, 1, 0);
  state.discard = {sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Preserve the prior GF plus Grass one-attachment recovery route.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
  expect(sim::EngineTestAccess::roseanne_finisher(fixture.engine) == sim::Card::Grass,
         "Roseanne regressed the original GF plus Grass recovery.");
}

void test_nonfinishing_basic_stays_rejected() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 1, 0);
  state.discard = {sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A single Grass here still leaves Apex Dragon one attachment short.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
  expect(!sim::EngineTestAccess::roseanne_finisher(fixture.engine).has_value(),
         "Roseanne accepted a recovered Basic that does not complete Apex.");
}

void test_recovery_is_unnecessary_when_same_basic_is_searchable() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 0, 1);
  state.discard = {sim::Card::Grass};
  state.deck = {sim::Card::Grass, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Earthen Vessel can already find the same Basic in deck, so spending the
  // Supporter on Roseanne would violate the route's resource-preservation policy.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Repository policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
  expect(!sim::EngineTestAccess::roseanne_finisher(fixture.engine).has_value(),
         "Roseanne recovered an Energy that was already searchable in deck.");
}

}  // namespace

int main() {
  try {
    test_dde_finisher(sim::Card::Grass);
    test_dde_finisher(sim::Card::Fire);
    test_basic_only_control();
    test_nonfinishing_basic_stays_rejected();
    test_recovery_is_unnecessary_when_same_basic_is_searchable();
    std::cout << "Issue 2444 Roseanne DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
