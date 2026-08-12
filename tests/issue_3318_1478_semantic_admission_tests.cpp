#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }
  static bool issue_3318_available(const Engine& engine) {
    return engine.issue_1478_t3_arven_vessel_completion_available();
  }
  static bool issue_3318_play(Engine& engine) {
    return engine.play_issue_1478_t3_arven_vessel_completion();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State ready_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn - 1, 1, 1,
                              sim::Tool::ForestSealStone};
  state.vstar_power_used = true;
  state.hand = {sim::Card::Arven, sim::Card::BrilliantBlender,
                sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::EarthenVessel, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Gladion,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool available(const sim::DciProfile dci, const sim::LockMode lock,
               const bool going_first, sim::State state, const int max_turn,
               const bool known = true) {
  const sim::Scenario scenario{"issue-3318", dci, lock, going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3318);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::issue_3318_available(engine);
}

void test_semantic_admission() {
  // These remaining actions are Trainer plays plus the normal attachment. Rule Box
  // Ability lock does not suppress them after the route's VSTAR Power is already spent.
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3318
  expect(available(sim::DciProfile::MatchupFlexJit,
                   sim::LockMode::FullRuleBoxAbility, true,
                   ready_state(4), 4),
         "MatchupFlex Rule Box Ability state hid legal completion");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None,
                   false, ready_state(2), 3),
         "going-second semantic state hid legal completion");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None,
                   true, ready_state(5), 5),
         "later semantic turn hid legal completion");

  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                    true, ready_state(3), 3),
         "NoDiscardControl entered same-ready-turn JIT packet");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                    true, ready_state(3), 3),
         "Item lock admitted Earthen Vessel route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                    true, ready_state(3), 3),
         "Supporter lock admitted Arven route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, ready_state(3), 3, false),
         "K0 admitted deterministic deck-search route");

  sim::State attachment_spent = ready_state(3);
  attachment_spent.manual_energy_used = true;
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, std::move(attachment_spent), 3),
         "spent manual attachment admitted completion");

  sim::State vstar_unspent = ready_state(3);
  vstar_unspent.vstar_power_used = false;
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, std::move(vstar_unspent), 3),
         "unspent route VSTAR state admitted completion");

  sim::State no_vessel = ready_state(3);
  no_vessel.deck.erase(std::remove(no_vessel.deck.begin(), no_vessel.deck.end(),
                                   sim::Card::EarthenVessel), no_vessel.deck.end());
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, std::move(no_vessel), 3),
         "missing Earthen Vessel admitted completion");

  sim::State no_payload = ready_state(3);
  no_payload.deck.erase(std::remove(no_payload.deck.begin(), no_payload.deck.end(),
                                    sim::Card::MegaDragonite), no_payload.deck.end());
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, std::move(no_payload), 3),
         "missing deck payload admitted completion");

  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, ready_state(4), 3),
         "turn beyond simulation horizon admitted completion");
}

void test_original_execution_still_works() {
  const sim::Scenario scenario{"issue-3318-execution", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3319);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, ready_state(3));
  expect(sim::EngineTestAccess::issue_3318_play(engine),
         "original semantic completion did not execute");
}

}  // namespace

int main() {
  test_semantic_admission();
  test_original_execution_still_works();
  return 0;
}
