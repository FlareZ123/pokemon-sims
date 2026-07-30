// Regression source: https://github.com/FlareZ123/pokemon-sims/issues/1879
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool route_available(const Engine& engine) {
    return engine.issue_1879_treasure_quick_ball_plan().has_value();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario() {
  return sim::Scenario{"issue-1879-matrix-boundaries",
                       sim::DciProfile::StrictJit,
                       sim::LockMode::None, true, 4};
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 2, 0, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::MysteriousTreasure,
      sim::Card::QuickBall,
      sim::Card::EarthenVessel,
      sim::Card::Fire,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
  };
  state.prizes = {
      sim::Card::BrilliantBlender,
      sim::Card::RegidragoVstar,
      sim::Card::Gladion,
      sim::Card::Serena,
      sim::Card::PathToPeak,
      sim::Card::FieldBlower,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  Fixture()
      : scenario_value(scenario()),
        recipe(sim::baseline_recipe()),
        rng(1879),
        trace{true, {}},
        engine(scenario_value, recipe, rng, &trace) {}
};

void unneeded_one_type_crispin_is_rejected() {
  Fixture fixture;
  sim::State state = base_state();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Only Grass is searchable, Fire is the sole missing GGF component, and Fire
  // is already held. Crispin cannot advance the Energy axis and must not consume
  // the Supporter slot or Bench space before the direct manual attachment:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Trainer no-effect ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // Official attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1879
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The route tried to play one-type Crispin for an unneeded Energy type.");
}

void payload_only_state_is_rejected() {
  Fixture fixture;
  sim::State state = base_state();
  state.active->fire = 1;
  state.deck.push_back(sim::Card::Fire);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // GGF is already complete, so Tapu Lele-GX and Crispin add no Energy value.
  // The specialized route must leave the payload-only state to ordinary Item
  // policy instead of forcing an ineffective Supporter sequence:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1879
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The Tapu-Crispin route preempted a payload-only state.");
}

}  // namespace

int main() {
  unneeded_one_type_crispin_is_rejected();
  payload_only_state_is_rejected();
  return 0;
}
