#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }

  static bool delayed_vessel(const Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn - 1, 1, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoV};
  return state;
}

bool visible(const sim::DciProfile dci, const sim::LockMode locks,
             const int turn, const int max_turn, const bool known = true,
             const bool attachment_spent = true, const bool payload = true,
             const bool vessel = true, const bool completing_energy = true) {
  std::mt19937_64 rng{3199};
  const sim::Scenario scenario{"issue-3199", dci, locks, false, max_turn};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::State state = route_state(turn);
  state.manual_energy_used = attachment_spent;
  if (!payload) state.hand.erase(state.hand.begin() + 1);
  if (!vessel) state.hand.erase(state.hand.begin());
  if (!completing_energy) {
    state.deck.erase(state.deck.begin());
  }
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::delayed_vessel(engine);
}

void test_semantic_positive_controls() {
  // Earthen Vessel discards the held Dragon on the projected ready turn, so both
  // same-ready-turn JIT profiles are equivalent for this route. Absolute T2 is a
  // historical witness coordinate; a later state with one turn of horizon remains
  // legal when Legacy Star and the projected Item action are available.
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Official Item, discard, search, attachment, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3199
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 3),
         "StrictJit historical route disappeared");
  expect(visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None, 2, 3),
         "MatchupFlexJit equivalent route was rejected");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None, 3, 4),
         "Later equivalent route was rejected");
}

void test_real_blockers_remain() {
  // Legacy Star is a Rule Box Pokemon Ability and the deferred Earthen Vessel is
  // an Item. Their actual legality must remain the lock boundary:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  expect(!visible(sim::DciProfile::StrictJit,
                  sim::LockMode::FullRuleBoxAbility, 2, 3),
         "Rule Box Ability lock illegally admitted Legacy Star");
  expect(!visible(sim::DciProfile::StrictJit,
                  sim::LockMode::TurnTwoItem, 2, 3),
         "Turn-two Item lock illegally admitted projected Vessel");
  expect(!visible(sim::DciProfile::StrictJit,
                  sim::LockMode::FullItem, 2, 3),
         "Full Item lock illegally admitted projected Vessel");
  expect(!visible(sim::DciProfile::StrictJit,
                  sim::LockMode::FullCombined, 2, 3),
         "Combined lock illegally admitted the route");
  expect(!visible(sim::DciProfile::NoDiscardControl,
                  sim::LockMode::None, 2, 3),
         "NoDiscardControl was conflated with same-ready-turn JIT");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 2),
         "Expired horizon admitted a deferred route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 3, false),
         "K0 state used hidden deck completion information");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 3,
                  true, false),
         "Unspent current attachment admitted the delayed-only route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 3,
                  true, true, false),
         "Missing held payload admitted the Vessel route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 3,
                  true, true, true, false),
         "Missing Vessel admitted the route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None, 2, 3,
                  true, true, true, true, false),
         "Missing completing Basic Energy admitted the route");
}
}  // namespace

int main() {
  try {
    test_semantic_positive_controls();
    test_real_blockers_remain();
    std::cout << "Issue 3199 delayed Vessel semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
