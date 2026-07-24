#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }

  static std::optional<Card> choose_discard(Engine& engine) {
    return engine.choose_discard(false, false, true, Card::QuickBall, false);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  const sim::Scenario scenario{"issue-1473", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const std::uint64_t seed, sim::State state)
      : rng(seed), engine(scenario, recipe, rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state));
  }
};

void test_vstar_and_backup_make_third_basic_discardable() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoV};
  Fixture fixture{1473, std::move(state)};

  // A Regidrago VSTAR publicly evolves from Regidrago V. The evolved stack and
  // a second public Regidrago V already establish two attacker lines, so a third
  // held Regidrago V is redundant dynamic-DCI fuel for Quick Ball:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Core evolution and public-board procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1473
  const auto selected = sim::EngineTestAccess::choose_discard(fixture.engine);
  if (!selected || *selected != sim::Card::RegidragoV) {
    throw std::runtime_error(
        "VSTAR plus backup Regidrago must make the third held Basic discardable.");
  }
}

void test_single_public_line_keeps_basic_protected() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoV};
  Fixture fixture{1474, std::move(state)};

  // With only the evolved attacker line public, the held Regidrago V remains the
  // sole backup Basic and must retain UDP/DCI protection:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1473
  if (sim::EngineTestAccess::choose_discard(fixture.engine)) {
    throw std::runtime_error(
        "A held Regidrago V must remain protected when only one public line exists.");
  }
}

void test_two_unevolved_lines_keep_existing_behavior() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoV};
  Fixture fixture{1475, std::move(state)};

  // The existing ordinary selector already permits a third Basic after two
  // unevolved Regidrago V are public. The VSTAR accounting fix must preserve it:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Existing selector source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_006.inc#L114-L153
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1473
  const auto selected = sim::EngineTestAccess::choose_discard(fixture.engine);
  if (!selected || *selected != sim::Card::RegidragoV) {
    throw std::runtime_error(
        "Two unevolved public Regidrago V must still make the third copy discardable.");
  }
}

}  // namespace

int main() {
  try {
    test_vstar_and_backup_make_third_basic_discardable();
    test_single_public_line_keeps_basic_protected();
    test_two_unevolved_lines_keep_existing_behavior();
    std::cout << "Issue 1473 redundant Regidrago V tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
