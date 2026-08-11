#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool final_energy_route(const Engine& engine) {
    return engine.arven_final_energy_vessel_dead_role_route_live();
  }
  static bool fss_blender_route(const Engine& engine) {
    return engine.arven_fss_blender_contention_route_live();
  }
  static bool held_arven_finish(const Engine& engine) {
    return engine.held_arven_fss_blender_finish_available();
  }
  static bool issue_2225_available(const Engine& engine) {
    return engine.issue_2225_arven_vessel_finish_available();
  }
  static bool issue_2225_play(Engine& engine) {
    return engine.play_issue_2225_arven_vessel_finish();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::Pokemon regi(const sim::Card card, const int grass, const int fire,
                  const int dde, const int entered_turn = 1) {
  sim::Pokemon result{card, entered_turn, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2426};
  sim::Engine engine;

  explicit Fixture(const sim::DciProfile dci = sim::DciProfile::StrictJit)
      : scenario{"issue-2426", dci, sim::LockMode::None, false, 5},
        engine(scenario, recipe, rng) {}
};

void test_final_energy_dead_role_accepts_dde_only(const sim::Card basic) {
  Fixture fixture(sim::DciProfile::NoDiscardControl);
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoVstar, 0, 0, 1);
  state.hand = {sim::Card::Arven, sim::Card::QuickBall};
  state.deck = {sim::Card::EarthenVessel, basic, sim::Card::Klara};
  state.discard = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One DDE supplies two all-type Energy on a Dragon. Either searchable Basic is
  // therefore one physical manual attachment from Apex Dragon, while Quick Ball
  // is dynamically dead once the setup and payload axes are complete:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/2426
  expect(sim::EngineTestAccess::final_energy_route(fixture.engine),
         "Arven final-Energy dead-role route rejected DDE plus one searchable Basic.");
}

void test_fss_blender_accepts_dde_basic(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoV,
                      basic == sim::Card::Grass ? 1 : 0,
                      basic == sim::Card::Fire ? 1 : 0, 1);
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::ForestSealStone, sim::Card::BrilliantBlender,
                sim::Card::RegidragoVstar, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The Energy stays attached through evolution. Arven may take Blender plus FSS,
  // FSS can find VSTAR, and Blender can establish the strict-JIT payload:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/2426
  expect(sim::EngineTestAccess::fss_blender_route(fixture.engine),
         "Arven FSS+Blender route rejected DDE plus Basic.");
  expect(sim::EngineTestAccess::held_arven_finish(fixture.engine),
         "Held Arven failed to suppress slower Steven for DDE plus Basic.");
}

void test_issue_2225_dde_only_finishes_with(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoVstar, 0, 0, 1);
  state.hand = {sim::Card::Arven, sim::Card::Dragapult};
  state.deck = {sim::Card::EarthenVessel, basic, sim::Card::Klara};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vessel's mandatory discard establishes the current-turn Dragon payload. The
  // searched Basic then remains in hand for the unused manual attachment to DDE:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2426
  expect(sim::EngineTestAccess::issue_2225_available(fixture.engine),
         "Arven->Vessel rejected DDE-only one-Basic completion.");
  expect(sim::EngineTestAccess::issue_2225_play(fixture.engine),
         "Arven->Vessel DDE route did not resolve.");
  const auto& after = sim::EngineTestAccess::state(fixture.engine);
  expect(std::count(after.hand.begin(), after.hand.end(), basic) == 1,
         "Vessel did not search the actual DDE-completing Basic.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::Dragapult) == 1,
         "Vessel did not spend the held Dragon as the strict-JIT payload cost.");
}

void test_basic_only_issue_2225_control() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoVstar, 1, 1, 0);
  state.hand = {sim::Card::Arven, sim::Card::Dragapult};
  state.deck = {sim::Card::EarthenVessel, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::issue_2225_available(fixture.engine),
         "Canonical GF plus Grass Arven->Vessel control regressed.");
}
}  // namespace

int main() {
  try {
    test_final_energy_dead_role_accepts_dde_only(sim::Card::Grass);
    test_final_energy_dead_role_accepts_dde_only(sim::Card::Fire);
    test_fss_blender_accepts_dde_basic(sim::Card::Grass);
    test_fss_blender_accepts_dde_basic(sim::Card::Fire);
    test_issue_2225_dde_only_finishes_with(sim::Card::Grass);
    test_issue_2225_dde_only_finishes_with(sim::Card::Fire);
    test_basic_only_issue_2225_control();
    std::cout << "issue-2426 Arven DDE semantics regressions passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
