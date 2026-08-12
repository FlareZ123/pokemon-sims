#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess3173Wonder {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }

  static bool visible(const Engine& engine) {
    return engine.issue_1796_wonder_tag_steven_route_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State wonder_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, turn - 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Grass};
  state.deck = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool visible(const sim::DciProfile dci, const sim::LockMode lock,
             const bool going_first, sim::State state, const int max_turn,
             const bool known = true) {
  const sim::Scenario scenario{"issue-3173-wonder-tag", dci, lock,
                               going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3173007);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess3173Wonder::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess3173Wonder::visible(engine);
}

void test_semantic_admission() {
  // Wonder Tag may search Steven's Resolve once K1 exposes the deterministic
  // next-turn package. StrictJit and MatchupFlexJit share same-ready-turn payload
  // timing, while seat and absolute turn add no card rule to this route:
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  expect(visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None,
                 true, wonder_state(2), 3),
         "MatchupFlexJit hid the Wonder Tag to Steven route");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 false, wonder_state(2), 3),
         "Going-second semantic state hid the Wonder Tag to Steven route");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 true, wonder_state(3), 4),
         "Equivalent later-turn Wonder Tag route was suppressed");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 false, wonder_state(1), 2),
         "Legal going-second T1 Wonder Tag bank was suppressed");
}

void test_semantic_boundaries() {
  // The route still needs both Rule Box Abilities, an Item on the next turn,
  // a legal Supporter play now, K1, an unused Retreat, and a next-turn horizon:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Turn-two Item lock starts on the player's second turn, so a T1 bank must
  // project Treasure legality to its T2 continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Scenario lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  expect(!visible(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                  true, wonder_state(2), 3),
         "NoDiscardControl entered the same-turn-JIT Wonder Tag route");
  expect(!visible(sim::DciProfile::StrictJit,
                  sim::LockMode::FullRuleBoxAbility, true,
                  wonder_state(2), 3),
         "Rule Box Ability lock admitted the Wonder Tag and Skyliner route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                  true, wonder_state(2), 3),
         "Item lock admitted the Treasure-dependent next turn");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem,
                  false, wonder_state(1), 2),
         "Scheduled T2 Item lock admitted a T1 bank whose Treasure is locked next turn");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                  true, wonder_state(2), 3),
         "Supporter lock admitted Wonder Tag into Steven");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, wonder_state(2), 3, false),
         "K0 exposed the deterministic Wonder Tag route");

  sim::State same_turn_regi = wonder_state(2);
  same_turn_regi.bench.front().entered_turn = 2;
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(same_turn_regi), 3),
         "Same-turn Regidrago V satisfied the evolution schedule");

  sim::State retreat_spent = wonder_state(2);
  retreat_spent.retreat_used = true;
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(retreat_spent), 3),
         "Spent Retreat admitted the Skyliner finish");

  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, wonder_state(2), 2),
         "Wonder Tag route ignored the next-turn horizon");
}

}  // namespace

int main() {
  test_semantic_admission();
  test_semantic_boundaries();
  return 0;
}
