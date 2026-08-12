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

  static bool initial_visible(const Engine& engine) {
    return engine.issue_1795_crispin_steven_vessel_route_available();
  }

  static bool banked_visible(Engine& engine) {
    engine.issue_1795_steven_turn_ = engine.state_.turn;
    return engine.issue_1795_banked_steven_route_available();
  }

  static bool finish(Engine& engine) {
    engine.issue_1795_vessel_turn_ = engine.state_.turn;
    return engine.complete_issue_1795_vessel_finish();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State initial_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MegaDragonite, sim::Card::Serena,
                sim::Card::StevensResolve, sim::Card::Gladion,
                sim::Card::Crispin, sim::Card::TapuLeleGX};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, turn - 1, 0, 0, sim::Tool::None}};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::QuickBall, sim::Card::MysteriousTreasure};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State banked_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::Fire, sim::Card::QuickBall};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State finish_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::QuickBall};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool visible(const sim::DciProfile dci, const sim::LockMode lock,
             const bool going_first, sim::State state, const int max_turn,
             const bool known = true) {
  const sim::Scenario scenario{"issue-3170", dci, lock, going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3170);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::initial_visible(engine);
}

void test_initial_semantics() {
  // Crispin, Steven's Resolve, and Earthen Vessel are Trainer actions. Rule Box
  // Ability lock leaves them legal, while both JIT profiles share ready-turn payload timing:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Advanced Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
  expect(visible(sim::DciProfile::MatchupFlexJit,
                 sim::LockMode::FullRuleBoxAbility, true,
                 initial_state(2), 4),
         "MatchupFlexJit Rule Box Ability state hid the legal route");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 false, initial_state(2), 4),
         "Equivalent going-second state was suppressed");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 true, initial_state(3), 5),
         "Equivalent later-turn state was suppressed");

  // NoDiscardControl has earlier payload timing and must stay outside this JIT route:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  expect(!visible(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                  true, initial_state(2), 4),
         "NoDiscardControl entered the same-turn-JIT route");

  // Earthen Vessel is an Item. Crispin and Steven's Resolve consume Supporter plays:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Advanced Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                  true, initial_state(2), 4),
         "Item lock admitted a Vessel-dependent schedule");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                  true, initial_state(2), 4),
         "Supporter lock admitted the Supporter schedule");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, initial_state(2), 4, false),
         "K0 exposed a K1 deterministic schedule");

  sim::State new_active = initial_state(2);
  new_active.active->entered_turn = 2;
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(new_active), 4),
         "Same-turn Regidrago V satisfied evolution timing");

  sim::State manual_spent = initial_state(2);
  manual_spent.manual_energy_used = true;
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(manual_spent), 4),
         "Spent manual attachment admitted the schedule");

  sim::State no_vstar = initial_state(2);
  no_vstar.deck.erase(std::remove(no_vstar.deck.begin(), no_vstar.deck.end(),
                                  sim::Card::RegidragoVstar),
                      no_vstar.deck.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_vstar), 4),
         "Missing VSTAR admitted the route");

  sim::State no_vessel = initial_state(2);
  no_vessel.deck.erase(std::remove(no_vessel.deck.begin(), no_vessel.deck.end(),
                                   sim::Card::EarthenVessel),
                       no_vessel.deck.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_vessel), 4),
         "Missing Vessel admitted the route");

  sim::State no_payload = initial_state(2);
  no_payload.hand.erase(std::remove(no_payload.hand.begin(), no_payload.hand.end(),
                                    sim::Card::MegaDragonite),
                        no_payload.hand.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_payload), 4),
         "Missing held payload admitted the route");

  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, initial_state(2), 3),
         "Route ignored the remaining two-turn horizon");
}

void test_continuation_semantics() {
  // The banked Steven and final Vessel steps retain the same Trainer/JIT semantics:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario banked_scenario{
      "issue-3170-banked", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::FullRuleBoxAbility, false, 6};
  std::mt19937_64 banked_rng(3171);
  sim::Engine banked_engine(banked_scenario, recipe, banked_rng);
  sim::EngineTestAccess::set_state(banked_engine, banked_state(4));
  expect(sim::EngineTestAccess::banked_visible(banked_engine),
         "Banked Steven continuation retained historical identity gates");

  const sim::Scenario finish_scenario{
      "issue-3170-finish", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::FullRuleBoxAbility, false, 6};
  std::mt19937_64 finish_rng(3172);
  sim::Engine finish_engine(finish_scenario, recipe, finish_rng);
  sim::EngineTestAccess::set_state(finish_engine, finish_state(5));
  expect(sim::EngineTestAccess::finish(finish_engine),
         "Semantic Vessel finish failed under equivalent later-turn Rule Box state");

  const sim::Scenario item_lock_scenario{
      "issue-3170-item-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullItem, true, 6};
  std::mt19937_64 lock_rng(3173);
  sim::Engine lock_engine(item_lock_scenario, recipe, lock_rng);
  sim::EngineTestAccess::set_state(lock_engine, banked_state(4));
  expect(!sim::EngineTestAccess::banked_visible(lock_engine),
         "Banked schedule ignored its future Earthen Vessel dependency");
}

}  // namespace

int main() {
  test_initial_semantics();
  test_continuation_semantics();
  return 0;
}
