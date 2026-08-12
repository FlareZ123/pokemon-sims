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
  }
  static void set_scenario(Engine& engine, Scenario scenario) {
    engine.scenario_ = std::move(scenario);
  }
  static void mark_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool route_visible(const Engine& engine) {
    return engine.issue_1646_vessel_burnet_finish_visible();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {
      sim::Card::ProfessorBurnet,
      sim::Card::EarthenVessel,
      sim::Card::QuickBall,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Dragapult,
      sim::Card::RegidragoV,
  };
  state.prizes = {
      sim::Card::ForestSealStone,
      sim::Card::FieldBlower,
      sim::Card::Oricorio,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::QuickBall,
  };
  return state;
}

bool visible_for(sim::Scenario scenario, const bool k1 = true) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3026};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::set_state(engine, route_state());
  sim::EngineTestAccess::set_scenario(engine, std::move(scenario));
  if (k1) sim::EngineTestAccess::mark_deck_seen(engine);
  return sim::EngineTestAccess::route_visible(engine);
}

void test_semantic_coordinates() {
  const auto rulebox =
      sim::scenario_by_label("strict-jit-rulebox-ability-lock/go-first");
  expect(rulebox.has_value(), "Rule Box Ability-lock scenario unavailable");

  // Earthen Vessel and Professor Burnet are Trainer cards. Rule Box Ability
  // lock does not stop either action, and T2 is a legal evolution/Trainer turn
  // after Regidrago V entered play previously. StrictJit and MatchupFlexJit use
  // the same ready-turn payload timing in repository policy:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official evolution, Item, Supporter, search, discard, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // JIT/K1/decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(visible_for(*rulebox), "T2 Rule Box Ability-lock route was rejected");

  sim::Scenario flex = *rulebox;
  flex.dci = sim::DciProfile::MatchupFlexJit;
  expect(visible_for(flex), "MatchupFlexJit route was rejected");

  sim::Scenario no_discard = *rulebox;
  no_discard.dci = sim::DciProfile::NoDiscardControl;
  expect(!visible_for(no_discard),
         "NoDiscardControl escaped strict-payload timing boundary");

  sim::Scenario full_item = *rulebox;
  full_item.locks = sim::LockMode::FullItem;
  expect(!visible_for(full_item), "Item lock did not block Earthen Vessel");

  expect(!visible_for(*rulebox, false), "K0 route inspected deck targets");
}

}  // namespace

int main() {
  try {
    test_semantic_coordinates();
    std::cout << "Issue 3026 semantic gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
