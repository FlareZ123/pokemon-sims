#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }

  static bool issue_1795_route_visible(Engine& engine) {
    return engine.issue_1795_crispin_steven_vessel_route_available();
  }

  static bool issue_1795_banked_visible(Engine& engine, const int turn) {
    engine.issue_1795_steven_turn_ = turn;
    return engine.issue_1795_banked_steven_route_available();
  }

  static bool issue_1795_finish(Engine& engine, const int turn) {
    engine.issue_1795_vessel_turn_ = turn;
    return engine.complete_issue_1795_vessel_finish_issue3170();
  }
};
}  // namespace sim

namespace {
void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile dci, const sim::LockMode lock,
                       const bool going_first, const int max_turn) {
  return sim::Scenario{"issue-3170", dci, lock, going_first, max_turn};
}

sim::State initial_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MegaDragonite, sim::Card::Serena,
                sim::Card::StevensResolve, sim::Card::Gladion,
                sim::Card::Crispin, sim::Card::TapuLeleGX};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0,
                              sim::Tool::None},
                 sim::Pokemon{sim::Card::TapuLeleGX, turn - 1, 0, 0,
                              sim::Tool::None}};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::QuickBall, sim::Card::MysteriousTreasure};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool initial_visible(const sim::DciProfile dci, const sim::LockMode lock,
                     const bool going_first, const int turn,
                     const int max_turn, const bool k1 = true) {
  const sim::Scenario value = scenario(dci, lock, going_first, max_turn);
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3170);
  sim::Engine engine(value, recipe, rng);
  sim::EngineTestAccess::set_state(engine, initial_state(turn), k1);
  return sim::EngineTestAccess::issue_1795_route_visible(engine);
}

sim::State banked_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 2, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::MegaDragonite, sim::Card::StevensResolve};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::EarthenVessel};
  state.prizes = {sim::Card::QuickBall, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State finish_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 2, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::MegaDragonite, sim::Card::RegidragoVstar,
                sim::Card::EarthenVessel, sim::Card::Grass};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::QuickBall, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

void initial_route_uses_semantic_admission() {
  // The printed route uses Supporters plus an Item and has no seat, absolute-turn,
  // DCI-profile, or Rule Box Ability-lock clause. Production still checks actual
  // Supporter/Item legality, K1, resource state, evolution age, and setup horizon.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // DCI and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
  require(initial_visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                          true, 2, 4),
          "Historical StrictJit #1795 route disappeared.");
  require(initial_visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None,
                          true, 2, 4),
          "MatchupFlexJit rejected the same-ready-turn route.");
  require(initial_visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                          false, 2, 4),
          "Seat identity rejected an equivalent legal route.");
  require(initial_visible(sim::DciProfile::StrictJit,
                          sim::LockMode::FullRuleBoxAbility, true, 2, 4),
          "Rule Box Ability lock rejected the Trainer-only route.");
  require(initial_visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                          true, 3, 5),
          "Absolute turn identity rejected an equivalent legal route.");
}

void invalid_routes_remain_rejected() {
  // NoDiscardControl deliberately uses a different payload timing policy. Item
  // lock blocks Vessel, Supporter lock blocks Crispin/Steven, K0 lacks the exact
  // schedule, and a short horizon cannot finish the two-turn continuation:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/3170
  require(!initial_visible(sim::DciProfile::NoDiscardControl,
                           sim::LockMode::None, true, 2, 4),
          "NoDiscardControl was admitted by the JIT-only route.");
  require(!initial_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::TurnTwoItem, true, 2, 4),
          "Item lock admitted an Earthen Vessel route.");
  require(!initial_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::FullSupporter, true, 2, 4),
          "Supporter lock admitted a Crispin/Steven route.");
  require(!initial_visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                           true, 2, 3),
          "Insufficient horizon admitted the route.");
  require(!initial_visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                           true, 2, 4, false),
          "K0 admitted an exact K1 schedule.");
}

void continuations_keep_the_same_semantics() {
  const sim::Scenario value = scenario(sim::DciProfile::MatchupFlexJit,
                                       sim::LockMode::FullRuleBoxAbility,
                                       false, 5);
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3171);
  sim::Engine banked(value, recipe, rng);
  sim::EngineTestAccess::set_state(banked, banked_state(3));
  require(sim::EngineTestAccess::issue_1795_banked_visible(banked, 3),
          "Banked Steven reintroduced historical coordinate gates.");

  std::mt19937_64 finish_rng(3172);
  sim::Engine finish(value, recipe, finish_rng);
  sim::EngineTestAccess::set_state(finish, finish_state(4));
  // Vessel discards the Dragon and the held Grass completes GGF in the same turn:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
  require(sim::EngineTestAccess::issue_1795_finish(finish, 4),
          "Vessel finish reintroduced historical coordinate gates.");
}
}  // namespace

int main() {
  try {
    initial_route_uses_semantic_admission();
    invalid_routes_remain_rejected();
    continuations_keep_the_same_semantics();
    std::cout << "Issue 3170 semantic route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
