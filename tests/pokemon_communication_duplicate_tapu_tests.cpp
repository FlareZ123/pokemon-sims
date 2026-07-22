#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static void set_deck_seen(Engine& engine) {
    engine.deck_seen_ = true;
  }

  static bool direct_route(Engine& engine, const Card target) {
    return engine.pokemon_communication_has_direct_target_route(target);
  }

  static std::optional<Engine::PokemonCommunicationPlan> plan(Engine& engine) {
    return engine.pokemon_communication_plan(false);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"pokemon-communication-duplicate-tapu",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{807};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State communication_state(const int tapu_copies) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::PokemonCommunication};
  for (int copy = 0; copy < tapu_copies; ++copy) {
    state.hand.push_back(sim::Card::TapuLeleGX);
  }
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Oricorio,
                sim::Card::LatiasEx, sim::Card::Grass};
  return state;
}

void test_duplicate_tapu_is_a_direct_exchange_for_each_missing_axis_target() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, communication_state(2));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Pokémon Communication may return either held Pokémon. Returning one of two
  // Tapu Lele-GX copies preserves the other copy's Wonder Tag role, so the direct
  // Item route remains live for every Pokémon axis checked by connector selection:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/807
  constexpr std::array<sim::Card, 3> targets{
      sim::Card::RegidragoVstar, sim::Card::Oricorio, sim::Card::LatiasEx};
  for (const sim::Card target : targets) {
    if (!sim::EngineTestAccess::direct_route(fixture.engine, target)) {
      throw std::runtime_error(
          "A redundant Tapu Lele-GX copy was not admitted as a direct Communication exchange.");
    }
  }

  const auto plan = sim::EngineTestAccess::plan(fixture.engine);
  if (!plan || plan->returned != sim::Card::TapuLeleGX ||
      plan->target != sim::Card::RegidragoVstar) {
    throw std::runtime_error(
        "The direct preflight and general Pokémon Communication planner still disagree.");
  }
}

void test_singleton_tapu_remains_protected() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, communication_state(1));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // The direct connector preflight must preserve a sole Tapu Lele-GX when that is
  // the only possible exchange Pokémon. The full play planner separately evaluates
  // whether Pokémon Communication itself makes Wonder Tag unnecessary:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/issues/807
  constexpr std::array<sim::Card, 3> targets{
      sim::Card::RegidragoVstar, sim::Card::Oricorio, sim::Card::LatiasEx};
  for (const sim::Card target : targets) {
    if (sim::EngineTestAccess::direct_route(fixture.engine, target)) {
      throw std::runtime_error(
          "A singleton Tapu Lele-GX was incorrectly exposed as a direct Communication exchange.");
    }
  }
}

}  // namespace

int main() {
  test_duplicate_tapu_is_a_direct_exchange_for_each_missing_axis_target();
  test_singleton_tapu_remains_protected();
  return 0;
}
