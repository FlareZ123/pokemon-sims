#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  sim::Scenario scenario{"issue-977-one-type-crispin", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const std::uint64_t seed = 977)
      : rng(seed), engine(scenario, recipe, rng) {}
};

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Dragapult,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Crispin, sim::Card::RegidragoV,
                  sim::Card::Grass, sim::Card::TapuLeleGX,
                  sim::Card::Serena, sim::Card::Channeler};
  return state;
}

void test_revealed_one_type_crispin_beats_redundant_regidrago_v() {
  // Gladion reveals every Prize before the exchange choice:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // Crispin may search one Basic Energy, put it into hand, and attach none. The
  // next turn's unused manual attachment then puts that Fire on the prior-turn GG
  // Regidrago V to complete Apex Dragon's GGF cost:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Existing one-type contract: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/crispin_one_type_manual_attachment_tests.cpp
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/977
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  expect(sim::EngineTestAccess::play_gladion(fixture.engine),
         "Gladion should resolve in the blind critical state");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::Crispin),
         "the revealed one-type Crispin route should enter hand");
  expect(contains(after.prizes, sim::Card::RegidragoV),
         "the redundant Regidrago V should remain in Prizes");
  expect(after.supporter_used, "Gladion should consume the Supporter play");
}

void test_missing_fire_keeps_crispin_dead() {
  // A one-type Crispin route advances Energy only when the required Fire remains in
  // the inspected deck: https://api.pokemontcg.io/v2/cards/sv7-133
  Fixture fixture{978};
  sim::State state = base_state();
  state.deck = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Crispin, sim::Card::RegidragoV,
                  sim::Card::Fire, sim::Card::TapuLeleGX,
                  sim::Card::Serena, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::play_gladion(fixture.engine),
         "Gladion should still resolve when the blind critical state is entered");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::Crispin),
         "Crispin must not be selected when Fire is absent from deck");
  expect(contains(after.hand, sim::Card::RegidragoV),
         "the ordinary fallback should remain available");
}

void test_used_manual_attachment_rejects_one_type_projection() {
  // One manual Energy attachment is permitted each turn. A one-type Crispin line
  // cannot finish GGF when that future attachment is unavailable:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  Fixture fixture{979};
  sim::State state = base_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::play_gladion(fixture.engine),
         "the legacy blind Gladion fallback should still resolve");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(after.hand, sim::Card::Crispin),
         "Crispin must remain prized when no manual attachment is available");
}

void test_prized_vstar_keeps_direct_missing_axis_priority() {
  // Direct recovery of the missing Regidrago VSTAR remains the first priority:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  Fixture fixture{980};
  sim::State state = base_state();
  state.deck = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Crispin, sim::Card::RegidragoVstar,
                  sim::Card::RegidragoV, sim::Card::Grass,
                  sim::Card::Serena, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::play_gladion(fixture.engine),
         "Gladion should recover the direct missing VSTAR axis");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "the prized VSTAR must outrank the one-type Crispin route");
  expect(!contains(after.hand, sim::Card::Crispin),
         "Crispin must remain prized when VSTAR is the direct missing axis");
}

}  // namespace

int main() {
  try {
    test_revealed_one_type_crispin_beats_redundant_regidrago_v();
    test_missing_fire_keeps_crispin_dead();
    test_used_manual_attachment_rejects_one_type_projection();
    test_prized_vstar_keeps_direct_missing_axis_priority();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
