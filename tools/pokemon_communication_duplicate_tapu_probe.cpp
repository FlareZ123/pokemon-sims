#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
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

int main() {
  const sim::Scenario scenario{"duplicate-tapu-communication-probe",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{20260717};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::PokemonCommunication,
                sim::Card::TapuLeleGX,
                sim::Card::TapuLeleGX};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // A duplicated Tapu Lele-GX is lower-marginal exchange material. Pokémon
  // Communication may return one copy and search Regidrago VSTAR while preserving
  // the other copy for Wonder Tag:
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  const bool direct = sim::EngineTestAccess::direct_route(
      engine, sim::Card::RegidragoVstar);
  const auto plan = sim::EngineTestAccess::plan(engine);
  const bool planner_accepts_duplicate_tapu = plan &&
      plan->returned == sim::Card::TapuLeleGX &&
      plan->target == sim::Card::RegidragoVstar;

  std::cout << "direct_route=" << direct << '\n'
            << "planner_route=" << planner_accepts_duplicate_tapu << '\n';

  // Current main is expected to reproduce the disagreement. A future fix should
  // invert this probe into a normal regression requiring both selectors to agree.
  return !direct && planner_accepts_duplicate_tapu ? 0 : 1;
}
