#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true; // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  }

  static bool held_quick_ball_route(const Engine& engine) {
    return engine.steven_held_routes_complete_next_turn(
        {Card::RegidragoVstar}, false, true);
  }

  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::Fire};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::BrilliantBlender, sim::Card::Oricorio,
                sim::Card::Grass};
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, std::mt19937_64& rng) {
  // Quick Ball discards another hand card before searching a Basic Pokémon, so the
  // held Dragon payload enters discard on the projected ready turn in both JIT profiles:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced turn/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-ready-turn JIT semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Resource-preserving route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3196
  const sim::Scenario scenario{"issue-3196-held-quick-ball-jit", dci,
                               sim::LockMode::None, false, 4};
  return sim::Engine(scenario, sim::baseline_recipe(), rng);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool helper_for(const sim::DciProfile dci) {
  std::mt19937_64 rng{3196};
  sim::Engine engine = make_engine(dci, rng);
  sim::EngineTestAccess::set_known_state(engine, route_state());
  return sim::EngineTestAccess::held_quick_ball_route(engine);
}

void test_same_ready_turn_profiles_share_held_quick_ball_route() {
  if (!helper_for(sim::DciProfile::StrictJit)) {
    throw std::runtime_error("StrictJit must admit the complete held-Quick-Ball route.");
  }
  if (!helper_for(sim::DciProfile::MatchupFlexJit)) {
    throw std::runtime_error("MatchupFlexJit must admit the same complete held-Quick-Ball route.");
  }
  if (helper_for(sim::DciProfile::NoDiscardControl)) {
    throw std::runtime_error("NoDiscardControl must stay outside the same-ready-turn JIT predicate.");
  }
}

void test_flex_steven_preserves_dominated_connectors() {
  std::mt19937_64 rng{3196};
  sim::Engine engine = make_engine(sim::DciProfile::MatchupFlexJit, rng);
  sim::EngineTestAccess::set_known_state(engine, route_state());

  if (!sim::EngineTestAccess::play_steven(engine)) {
    throw std::runtime_error("Steven must resolve in the confirmed MatchupFlexJit state.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Steven must reserve the missing Regidrago VSTAR.");
  }
  if (!contains(after.deck, sim::Card::Crispin) ||
      !contains(after.deck, sim::Card::BrilliantBlender)) {
    throw std::runtime_error("The held Quick Ball route must not spend Steven slots on redundant connectors.");
  }
  if (!contains(after.hand, sim::Card::QuickBall) ||
      !contains(after.hand, sim::Card::MegaDragonite) ||
      !contains(after.hand, sim::Card::Fire)) {
    throw std::runtime_error("Steven must preserve the held payload, Quick Ball, and final Energy route.");
  }
}

}  // namespace

int main() {
  try {
    test_same_ready_turn_profiles_share_held_quick_ball_route();
    test_flex_steven_preserves_dominated_connectors();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
